#include "Assembler.h"
#include <string>
#include <iostream>
#include <regex>
#include "Constants.h"
#include "Utility.h"
#include <csignal>

Assembler::Assembler(string inputFileName, string outputFileName) {
	this->inputFileName = inputFileName;
	this->outputFileName = outputFileName;	
}

Assembler::~Assembler() {
	delete reader;

	sectionArray.clear();
}

string Assembler::getErrorDescription() {
	return errorDescription;
}

void Assembler::resetSectionCounters() {
	for (vector<Section*>::iterator it = sectionArray.begin(); it != sectionArray.end(); ++it) {
		(*it)->resetLocationCounter();
	}
}

bool Assembler::sectionExists(Section section) {
	for (vector<Section*>::iterator it = sectionArray.begin(); it != sectionArray.end(); ++it) {
		if (!((*it)->getName()).compare(section.getName())) {
			return true;
		}
	}
	return false;
}

Section* Assembler::findSectionByOrdNumber(unsigned int orderNumber) {
	for (vector<Section*>::iterator it = sectionArray.begin(); it != sectionArray.end(); ++it) {
		if ((*it)->getOrderNumber() == orderNumber) {
			return *it;
		}
	}
	return nullptr;
}

Section* Assembler::findSectionByName(string name) {
	for (vector<Section*>::iterator it = sectionArray.begin(); it != sectionArray.end(); ++it) {
		if (!((*it)->getName()).compare(name)) {
			return *it;
		}
	}
	return nullptr;
}

void Assembler::writeSectionContent(string content) {
	currentSection->writeSectionContent(content);
}

long long Assembler::calculateExpression(string symbol, string operation, string infix) {
	unsigned int sectionNumber = 0;
	unsigned long address = 0;

	unsigned long long secondDoubleWord = 0;
	unsigned long long offset = 0;

	vector<string> postfix;

	Symbol* entry = (Symbol*)symbolTable.findSymbolByName(symbol);

	if (entry != nullptr) {
		offset = entry->getSectionOffset();
		sectionNumber = entry->getSectionNumber();	
		Section* section = findSectionByOrdNumber(sectionNumber);
		address = section->getStartAddress() + offset;
	}

	long result = 0;

	if (infix.compare("")) {
		postfix = infixToPostfixExpression(infix);
		result = evaluateExpression(postfix);
	}
		
	if (!operation.compare("+")) {
		secondDoubleWord = address + result;
	} 
	else if (!operation.compare("-")) {
		secondDoubleWord = address - result;
	}
	else {
		if (symbol.compare("")) {
			secondDoubleWord = address;
		}
		else {
			secondDoubleWord = result;
		}
	}

	return secondDoubleWord;
}

unsigned long Assembler::createCodeRegisterDirect(vector<string> arguments, int instructionCode, int type) {
	unsigned long addressModeCode = REG_DIR_ADDR_MODE;
	regex rgx("(R1[0-5]{1}|R[0-9]{1}|PC|SP){1}");

	unsigned long machineCode = 0;
	unsigned long registerCode = 0;
	
	for (int i = 0; i < arguments.size(); i++) {	
		smatch match;
		
		const string str = arguments[i];
		string registerOperand = "";

		if (regex_search(str.begin(), str.end(), match, rgx))
			registerOperand = match[1];

		registerCode = registerCodes.at(registerOperand);
		machineCode |= (registerCode << (REG0_OFFSET - 5 * i));
	}

	machineCode |= (instructionCode << OPCODE_OFFSET);
	machineCode |= (type << TYPE_OFFSET);
	machineCode |= (addressModeCode << ADDR_MODE_OFFSET);

	return machineCode;
}

unsigned long Assembler::createCodeRegisterIndirect(vector<string> arguments, int instructionCode, int type) {
	unsigned long addressModeCode = REG_IND_ADDR_MODE;
	regex rgx("\\[(R1[0-5]{1}|R[0-9]{1}|PC|SP){1}\\]");

	unsigned long machineCode = 0;
	unsigned long registerCode = 0;
	
	for (int i = 0; i < arguments.size(); i++) {	
		smatch match;
		
		const string str = arguments[i];
		string registerOperand = "";

		if (regex_search(str.begin(), str.end(), match, rgx))
			registerOperand = match[1];

		registerCode = registerCodes.at(registerOperand);

		if (instructionCode == 0 || instructionCode == 2 || instructionCode == 3) { 
			machineCode |= (registerCode << (REG0_OFFSET - 5 * i));
		}
		else {
			machineCode |= (registerCode << (REG1_OFFSET - 5 * i));
		}
	}

	machineCode |= (instructionCode << OPCODE_OFFSET);
	machineCode |= (type << TYPE_OFFSET);
	machineCode |= (addressModeCode << ADDR_MODE_OFFSET);

	return machineCode;
}

unsigned long long Assembler::createCodeRegisterIndirectDisplacement(string argument, int codeInstruction, int type) {
	unsigned long long machineCode = 0;
	
	unsigned long long firstDoubleWord = 0;
	long long  secondDoubleWord = 0;
	
	unsigned long registerCode = 0;
	
	unsigned long addressModeCode = REG_IND_DISP_ADDR_MODE;
	
	regex rgx("\\[((R[0-9]{1}|1[0-5])|PC|SP){1}(\\s)*\\+(\\s)*(.*)\\]");
	smatch match;

	const string str = argument;
	string registerOperand = "";
	string displacement = "";

	if (regex_search(str.begin(), str.end(), match, rgx)) {
		registerOperand = match[1];
		displacement = match[5];
	}
	
	string infix;
	string symbol;
	string operation;

	const string stringDisplacement = displacement;
									
	if (regex_search(stringDisplacement.begin(), stringDisplacement.end(), match, REGEX_CONST_EXPRESSION)) {
		symbol = match [1];
		operation = match[3]; 
		infix = match [4];
	}
	
	secondDoubleWord = calculateExpression(symbol, operation, infix);
	
	Symbol* entry = (Symbol*)symbolTable.findSymbolByName(symbol);

	if (entry != nullptr) {
		unsigned int sectionNumber = entry->getSectionNumber();	
		Section* section = findSectionByOrdNumber(sectionNumber);
		
		string flags = section->getFlags();
		
		if (flags.find_first_of("F") == string::npos) {
			RelocationTableEntry relocationEntry;

			relocationEntry.setType('A');
			relocationEntry.setAddress(currentSection->getStartAddress() + currentSection->getLocationCounter() + 4);
			relocationEntry.setOrderNumber(section->getOrderNumber());

			currentSection->addRelocationRecord(relocationEntry);
		}
	}

	registerCode = registerCodes[registerOperand];
	
	firstDoubleWord |= (codeInstruction << OPCODE_OFFSET);
	firstDoubleWord |= (addressModeCode << ADDR_MODE_OFFSET);
	firstDoubleWord |= (type << TYPE_OFFSET);
	
	//	INT, JMP or CALL instructions (has only one argument)
	if (codeInstruction == 0 || codeInstruction == 2 || codeInstruction == 3) {
		firstDoubleWord |= (registerCode << REG0_OFFSET);
	} else {
		firstDoubleWord |= (registerCode << REG1_OFFSET);
	} 
		
	machineCode |= secondDoubleWord;
	machineCode |= (firstDoubleWord << 32);
	
	return machineCode;
}

unsigned long long Assembler::createCodeImmediate(string argument, int codeInstruction, int type) {
	unsigned long long machineCode = 0;
	
	unsigned long long firstDoubleWord = 0;
	unsigned long long  secondDoubleWord = 0;

	unsigned long addressModeCode = IMMEDIATE_ADDR_MODE;
	
	regex rgx("\\#([a-z|A-Z|_][a-zA-Z0-9_]*){0,1}([ ]*){0,1}([\\+\\-]){0,1}(.*){0,1}");
	smatch match;
		
	string infix;
	string symbol;
	string operation;

	const string str = argument;
									
	if (regex_search(str.begin(), str.end(), match, rgx)) {
		symbol = match [1];
		operation = match[3]; 
		infix = match [4];
	}
	
	secondDoubleWord = calculateExpression(symbol, operation, infix);

	Symbol* entry = (Symbol*)symbolTable.findSymbolByName(symbol);

	if (entry != nullptr) {
		unsigned int sectionNumber = entry->getSectionNumber();	
		Section* section = findSectionByOrdNumber(sectionNumber);
		
		string flags = section->getFlags();
		
		if (flags.find_first_of("F") == string::npos) {
			RelocationTableEntry relocationEntry;

			relocationEntry.setType('A');
			relocationEntry.setAddress(currentSection->getStartAddress() + currentSection->getLocationCounter() + 4);
			relocationEntry.setOrderNumber(section->getOrderNumber());

			currentSection->addRelocationRecord(relocationEntry);
		}
	}
	
	firstDoubleWord |= (codeInstruction << OPCODE_OFFSET);
	firstDoubleWord |= (addressModeCode << ADDR_MODE_OFFSET);
	firstDoubleWord |= (type << TYPE_OFFSET);

	machineCode |= secondDoubleWord;
	machineCode |= (firstDoubleWord << 32);

	return machineCode;
}

unsigned long long Assembler::createCodeMemoryDirect(string argument, int codeInstruction, int type) {
	unsigned long long machineCode = 0;
	
	unsigned long long firstDoubleWord = 0;
	unsigned long long  secondDoubleWord = 0;

	unsigned long addressModeCode = MEM_DIR_ADDR_MODE;
	
	regex rgx("([a-z|A-Z|_][a-zA-Z0-9_]*){0,1}([ ]*){0,1}([\\+\\-]){0,1}(.*){0,1}");
	smatch match;
		
	string infix;
	string symbol;
	string operation;

	const string str = argument;
									
	if (regex_search(str.begin(), str.end(), match, rgx)) {
		symbol = match [1];
		operation = match[3]; 
		infix = match [4];
	}
	
	secondDoubleWord = calculateExpression(symbol, operation, infix);
	
	Symbol* entry = (Symbol*)symbolTable.findSymbolByName(symbol);

	if (entry != nullptr) {
		unsigned int sectionNumber = entry->getSectionNumber();	
		Section* section = findSectionByOrdNumber(sectionNumber);
		
		string flags = section->getFlags();
		
		if (flags.find_first_of("F") == string::npos) {
			RelocationTableEntry relocationEntry;

			relocationEntry.setType('A');
			relocationEntry.setAddress(currentSection->getStartAddress() + currentSection->getLocationCounter() + 4);
			relocationEntry.setOrderNumber(section->getOrderNumber());

			currentSection->addRelocationRecord(relocationEntry);
		}
	}
	
	firstDoubleWord |= (codeInstruction << OPCODE_OFFSET);
	firstDoubleWord |= (addressModeCode << ADDR_MODE_OFFSET);
	firstDoubleWord |= (type << TYPE_OFFSET);

	machineCode |= secondDoubleWord;
	machineCode |= (firstDoubleWord << 32);

	return machineCode;
}

unsigned long long Assembler::createCodePCRelative(string argument, int codeInstruction, int type) {
	unsigned long long machineCode = 0;
	
	unsigned long long firstDoubleWord = 0;
	unsigned long long  secondDoubleWord = 0;
	
	unsigned long registerCode = 0;
	unsigned long addressModeCode = REG_IND_DISP_ADDR_MODE;
	
	regex rgx("\\$([a-z|A-Z|_][a-zA-Z0-9_]*|0x[0-9]{0,8})");
	smatch match;
	
	regex rgxAddress("(0x[0-9]{0,8})");
	smatch matchAddress;
	
	const string str = argument;
	unsigned int address = 0;
	string symbol = "";
	
	if (regex_search(str.begin(), str.end(), match, rgx)) {
		const string symbolAddress = match[1];
		
		if (regex_search(symbolAddress.begin(), symbolAddress.end(), matchAddress, rgxAddress)) {
			address = stoi(matchAddress[1], nullptr, 0);
		} else {
			symbol = symbolAddress;
		}
	}
	
	Symbol* entry = (Symbol*)symbolTable.findSymbolByName(symbol);

	if (entry != nullptr) {
		unsigned int sectionNumber = entry->getSectionNumber();	
		Section* section = findSectionByOrdNumber(sectionNumber);
		
		if (section != currentSection){
			string flags = section->getFlags();
			
			address = entry->getValue();

			if (flags.find_first_of("F") == string::npos) {
				RelocationTableEntry relocationEntry;
	
				relocationEntry.setType('R');
				relocationEntry.setAddress(currentSection->getStartAddress() + currentSection->getLocationCounter() + 4);
				relocationEntry.setOrderNumber(section->getOrderNumber());

				currentSection->addRelocationRecord(relocationEntry);
			}
		} else {
			address = entry->getValue() - currentSection->getStartAddress() - currentSection->getLocationCounter() - 8;
		}

	}

	registerCode = registerCodes["PC"];
	
	firstDoubleWord |= (codeInstruction << OPCODE_OFFSET);
	firstDoubleWord |= (addressModeCode << ADDR_MODE_OFFSET);
	firstDoubleWord |= (registerCode << REG1_OFFSET);
	firstDoubleWord |= (type << TYPE_OFFSET);
	
	secondDoubleWord = address;
	
	machineCode |= secondDoubleWord;
	machineCode |= (firstDoubleWord << 32);

	return machineCode;
}

bool Assembler::firstPass() {
	if (!isFileExists(inputFileName)) {
		errorDescription = "ERROR: File '" + inputFileName + "' doesn't exists.";
		return false;
	}

	reader = new Reader(inputFileName);

	error = false;
	
	string str;

	int lineCounter = 1;
	int orderNumberCounter = 1;
	int incrementer;

	while (1) {
		cout << "==================================================" << endl;

		str = reader->readNextLine();

		if (str.empty()) {
			error = true;
			errorDescription = "There is not .end directive.";

			return !error;
		}

		str = reader->trim(str);
		str = reader->discardComment(str);

		string word = reader->getFirstWord(str);

		if (!word.compare(".end")) {
			cout << "First Pass finised. End of assembler file." << endl;
			break;
		}

		if (reader->isInstruction(word) || reader->isLabel(word)) {
			if (reader->isLabel(word)) {
				string nextWord = reader->getFirstWord(str);

				if (symbolTable.findSymbolByName(word) == nullptr) {
					Symbol *symbol = new Symbol();

					symbol->setType("SYM");
					symbol->setOrderNumber(orderNumberCounter++);
					symbol->setName(word.substr(0, word.size() - 1));
					symbol->setSectionNumber(currentSection->getOrderNumber());
					symbol->setValue(currentSection->getLocationCounter());
					symbol->setSectionOffset(currentSection->getLocationCounter());
					symbol->setFlag('L');

					symbolTable.addSymbol(symbol);
				}

				if (!nextWord.empty()) {
					if (find(mnemonics.begin(), mnemonics.end(), nextWord) != mnemonics.end()) {
						word = nextWord;
					}
					else if (find(dataDefining.begin(), dataDefining.end(), nextWord) != dataDefining.end()) {
						cout << "THIS IS LABEL WITH DATA DEFINITION " << nextWord << endl;
					}
				}
				else {
					cout << "THIS IS JUST LABEL " << endl;
				}
			}
			
			if (reader->isInstruction(word)) {
				vector<string> arguments = reader->split(str, ',');

				int argumentsNumber = arguments.size();
				string instruction = reader->trim(word);

				if (reader->isControlFlowInstruction(instruction)) {
					cout << "CONTROL FLOW INSTRUCTION: " << word << endl;

					if (!instruction.compare("INT")) {
						incrementer = 4;
						
						if (argumentsNumber != 1) {
							error = true;
							errorDescription =  "ERROR Line " + to_string(lineCounter) + ": Unexcepted arguments number. INT excepts 1 argument.";
							
							return !error;
						}
					}

					if (!instruction.compare("JMP") || !instruction.compare("CALL")) {
						if (argumentsNumber != 1) {
							error = true;
							errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted arguments number. " + instruction + " excepts 1 argument.";
							
							return !error;
						}

						for (vector<string>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
							*it = reader->trim(*it);

							if (regex_match(*it, REGEX_ADDR_MODE_MEM_DIR) ||
								regex_match(*it, REGEX_ADDR_MODE_REG_IND_DISP) ||
								regex_match(*it, REGEX_ADDR_MODE_DOLLAR_PC)) {
								incrementer = 8;
							}
						    else if(regex_match(*it, REGEX_ADDR_MODE_REG_IND)) {
								incrementer = 4;
							}
							else {
								error = true;
								errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted address mode.";
								
								return !error;
							}
						}
					}

					if (!instruction.compare("RET") || !instruction.compare("RTI")) {
						incrementer = 4;

						if (argumentsNumber != 1 && arguments.at(0) != "") {
							error = true;
							errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted arguments number. " + instruction + " excepts 0 argument.";
							
							return !error;
						}
					}

					if (!instruction.compare("JZ")
						|| !instruction.compare("JNZ")
						|| !instruction.compare("JGZ")
						|| !instruction.compare("JGEZ")
						|| !instruction.compare("JLZ")
						|| !instruction.compare("JLEZ")) {

						if (argumentsNumber != 2) {
							error = true;
							errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted arguments number. " + instruction + " excepts 2 argument.";
							
							return !error;
						}

						if (!regex_match(reader->trim(arguments.at(0)), REGEX_ADDR_MODE_REG_DIR)) {
							error = true;
							errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted address mode for first argument.";
							
							return !error;
						}

						if (regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_MEM_DIR) ||
							regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND_DISP) ||
							regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_DOLLAR_PC)) {
							incrementer = 8;
						} 
						else if (regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND)) {
							incrementer = 4;
						} 
						else {
							error = true;
							errorDescription =  "ERROR Line " + to_string(lineCounter) + ": Unexcepted address mode for second argument.";

							return !error;
						}
					}

					currentSection->incrementLocationCounterBy(incrementer);
				}
				else if (reader->isLoadStoreInstruction(instruction)) {
					cout << "LOAD STORE INSTRUCTION" << endl;

					if (argumentsNumber != 2) {
						error = true;
						errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted arguments number. " + instruction + " excepts 2 argument.";
						
						return !error;
					}

					if (!regex_match(reader->trim(arguments.at(0)), REGEX_ADDR_MODE_REG_DIR)) {
						error = true;
						errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted address mode for first argument.";

						return !error;
					}

					if (!instruction.compare("LOAD") ||
						!instruction.compare("LOADSB") ||
						!instruction.compare("LOADUW") ||
						!instruction.compare("LOADSW")) {

						if (regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_IMMEDIATE) ||
							regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND_DISP) ||
							regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_MEM_DIR) ||
							regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_DOLLAR_PC)) {
							incrementer = 8;
						}
						else if (regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_DIR) ||
							     regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND)) {
							incrementer = 4;
						} 
						else {
							
							error = true;
						 	errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted address mode for second argument.";
							
							return !error;
						}
					}

					if (!instruction.compare("STORE") ||
						!instruction.compare("STOREB") ||
						!instruction.compare("STOREW")) {

						if (regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_IMMEDIATE) ||
							regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND_DISP) ||
							regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_MEM_DIR) ||
							regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_DOLLAR_PC)) {
							incrementer = 8;
						}
						else if (regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_DIR) ||
							     regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND)) {
							incrementer = 4;
						} 
						else {
							error = true;
							errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted address mode for second argument.";
							
							return !error;
						}
					}

					currentSection->incrementLocationCounterBy(incrementer);
				}
				else if (reader->isStackInstruction(instruction)) {
					incrementer = 4;

					if (argumentsNumber != 1) {
						error = true;
						errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted arguments number. " + instruction + " excepts 1 argument.";
						
						return !error;
					}

					if (!regex_match(reader->trim(arguments.at(0)), REGEX_ADDR_MODE_REG_DIR)) {
						error = true;
						errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted address mode for first argument.";
						
						return !error;
					}

					currentSection->incrementLocationCounterBy(incrementer);
				}
				else if (reader->isAritmeticLogicInstruction(instruction)) {
					incrementer = 4;

					if (instruction == "NOT") {
						if (argumentsNumber != 2) {
							error = true;
							errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted arguments number. " + instruction + " excepts 2 argument.";
							
							return !error;
						}
					}
					else {
						if (argumentsNumber != 3) {
							error = true;
							errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted arguments number. " + instruction + " excepts 3 argument.";
							
							return !error;
						}
					}

					for (vector<string>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
						if (!regex_match(reader->trim(*it), REGEX_ADDR_MODE_REG_DIR)) {
							error = true;						
							errorDescription = "ERROR Line " + to_string(lineCounter) + ": Unexcepted address mode for argument.";
							
							return !error;
						}
					}

					currentSection->incrementLocationCounterBy(incrementer);
				}
			}

			
		}
		else if (reader->isSection(word)) {
			cout << "THIS IS SECTION WITH NAME: " << word << endl;

			if (symbolTable.findSymbolByName(word) == nullptr) {
				Section *section = new Section();
				
				section->setType("SEG");
				section->setName(word);
				section->setOrderNumber(orderNumberCounter);
				section->setSectionNumber(orderNumberCounter++);
				section->setSectionOffset(0);
				section->resetLocationCounter();
				
				currentSection = section;
				
				sectionArray.push_back(section);
				
				symbolTable.addSymbol(section);
			}
		}
		lineCounter++;
	}
	
	resetSectionCounters();

	return true;
}

bool Assembler::secondPass() {
	string str;

	int lineCounter = 1;
	
	unsigned long long machineCode = 0;

	unsigned long long firstDoubleWord = 0;

	unsigned long long instructionCode = 0;
	unsigned long long addressModeCode = 0;
	unsigned long long firstRegisterCode = 0;
	unsigned long long secondRegisterCode = 0;
	unsigned long long thirdRegisterCode = 0;

	unsigned long long secondDoubleWord;

	bool previousLineOrg = false;
	unsigned long orgAddress = 0;

	error = false;

	while (1) {
		cout << "==================================================" << endl;
		
		str = reader->readNextLine();

		if (str.empty()) {
			error = true;
			errorDescription = "There is not .end directive.";
			
			return !error;
		}

		str = reader->trim(str);
		str = reader->discardComment(str);

		string word = reader->getFirstWord(str);
		


		if (!word.compare(".end")) {
			cout << "Second Pass finised. End of assembler file." << endl;
			break;
		}

		if (reader->isInstruction(word) || reader->isLabel(word)) {
			if (reader->isLabel(word)) {
				string nextWord = reader->getFirstWord(str);

				Symbol* symbol = (Symbol*)symbolTable.findSymbolByName(word.substr(0, word.size() - 1));

				if (symbol != nullptr) {
					symbol->setValue(currentSection->getStartAddress() + currentSection->getLocationCounter());
					symbol->setSectionOffset(symbol->getValue() - currentSection->getStartAddress());
				}

				if (!nextWord.empty()) {
					if (find(mnemonics.begin(), mnemonics.end(), nextWord) != mnemonics.end()) {
						word = nextWord;
					}
					else if (find(dataDefining.begin(), dataDefining.end(), nextWord) != dataDefining.end()) {
						cout << "THIS IS LABEL WITH DATA DEFINITION " << nextWord << endl;
						
						vector<string> dataDefineFields = reader->split(str, ',');
						
						int dataSize = 0;

						if (!nextWord.compare("DB")) {
							dataSize = 1;
						}

						if (!nextWord.compare("DW")) {
							dataSize = 2;
						}

						if (!nextWord.compare("DD")) {
							dataSize = 4;
						}

						for (vector<string>::iterator it = dataDefineFields.begin(); it != dataDefineFields.end(); ++it) {
							*it = reader->trim(*it);
							
							smatch match;
							string hexstring;

							if (it->find_first_of(" ") != string::npos) {
								vector<string> dupArguments = reader->split(*it, ' ');
								
								string firstArgument = dupArguments.at(0);
								string secondArgument = dupArguments.at(1);
								string thirdArgument = dupArguments.at(2);
								
								unsigned long long firstArgumentNumber = stoi(firstArgument, nullptr, 0);
								unsigned long long secondArgumentNumer = stoi(thirdArgument, nullptr, 0);

								hexstring = longlongToHexString(secondArgumentNumer, dataSize);
							
								for (int i = 0; i < firstArgumentNumber; i++) {
									currentSection->writeSectionContent(hexstring);
								}
								
								currentSection->incrementLocationCounterBy(dataSize * firstArgumentNumber);

							} else {
								const string str = *it;
								string symbol;
								string operation;
								string infix;
	
								if (regex_search(str.begin(), str.end(), match, REGEX_CONST_EXPRESSION)) {
									symbol = match [1];
									operation = match[3]; 
									infix = match [4];
								}
								
								unsigned long long argument = calculateExpression(symbol, operation, infix);
	
								hexstring = longlongToHexString(argument, dataSize);

								currentSection->writeSectionContent(hexstring);

								Symbol* entry = (Symbol*)symbolTable.findSymbolByName(symbol);
								
								if (entry != nullptr) {
									unsigned int sectionNumber = entry->getSectionNumber();	
									Section* section = findSectionByOrdNumber(sectionNumber);
									
									string flags = section->getFlags();
									
									if (flags.find_first_of("F") == string::npos) {
										RelocationTableEntry relocationEntry;
							
										relocationEntry.setType('A');
										relocationEntry.setAddress(currentSection->getStartAddress() + currentSection->getLocationCounter());
										relocationEntry.setOrderNumber(section->getOrderNumber());
						
										currentSection->addRelocationRecord(relocationEntry);
									}
								}

								currentSection->incrementLocationCounterBy(dataSize);
							}
						}
					}
				}
				else {
					cout << "THIS IS JUST LABEL " << endl;
				}
			}
			
			if (reader->isInstruction(word)) {
				vector<string> arguments = reader->split(str, ',');
				string instruction = word;

				instructionCode = instructions[instruction];

				if (reader->isControlFlowInstruction(instruction)) {
					cout << "CONTROL FLOW INSTRUCTION: " << word << endl;

					if (!instruction.compare("INT") || !instruction.compare("RET") || !instruction.compare("RTI")) {
						firstDoubleWord = instructionCode << OPCODE_OFFSET;

						string hexCode = longlongToHexString(firstDoubleWord, 4);

						writeSectionContent(hexCode);

						currentSection->incrementLocationCounterBy(4);
					}
					
					if (!instruction.compare("JMP") || !instruction.compare("CALL")) {

						for (vector<string>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
							*it = reader->trim(*it);

							if (regex_match(*it, REGEX_ADDR_MODE_MEM_DIR)) {
								machineCode = createCodeMemoryDirect(*it, instructionCode, 0);
								
								string hexCode = longlongToHexString(machineCode, 8);
								
								writeSectionContent(hexCode);
								
								currentSection->incrementLocationCounterBy(8);
							}
							else if (regex_match(*it, REGEX_ADDR_MODE_REG_IND)) {
								vector<string> arg;
								arg.push_back(*it);

								firstDoubleWord = createCodeRegisterIndirect(arg, instructionCode, 0);
							
								string hexCode = longlongToHexString(firstDoubleWord, 4);

								writeSectionContent(hexCode);

								currentSection->incrementLocationCounterBy(4);
							}
							else if (regex_match(*it, REGEX_ADDR_MODE_REG_IND_DISP)) {
								machineCode = createCodeRegisterIndirectDisplacement(*it, instructionCode, 0);
								
								string hexCode = longlongToHexString(machineCode, 8);
								
								writeSectionContent(hexCode);
								
								currentSection->incrementLocationCounterBy(8);
							}
							else if (regex_match(*it, REGEX_ADDR_MODE_DOLLAR_PC)) {
								machineCode = createCodePCRelative(*it, instructionCode, 0);
								
								string hexCode = longlongToHexString(machineCode, 8);
								
								writeSectionContent(hexCode);
								
								currentSection->incrementLocationCounterBy(8);
							}
						}

					}

					if (!instruction.compare("JZ")
						|| !instruction.compare("JNZ")
						|| !instruction.compare("JGZ")
						|| !instruction.compare("JGEZ")
						|| !instruction.compare("JLZ")
						|| !instruction.compare("JLEZ")) {

						string firstArgument = reader->trim(arguments.at(0));

						if (regex_match(firstArgument, REGEX_ADDR_MODE_REG_DIR)) {
							vector<string> arg;
							arg.push_back(firstArgument);

							firstDoubleWord = createCodeRegisterDirect(arg, instructionCode, 0);
						}

						string secondArgument = reader->trim(arguments.at(1));

						if (regex_match(secondArgument, REGEX_ADDR_MODE_MEM_DIR)) {
							machineCode = (firstDoubleWord << 32) | createCodeMemoryDirect(secondArgument, instructionCode, 0);
							
							string hexCode = longlongToHexString(machineCode, 8);
							
							writeSectionContent(hexCode);
							
							currentSection->incrementLocationCounterBy(8);
						}
						else if (regex_match(secondArgument, REGEX_ADDR_MODE_REG_IND)) {
							vector<string> arg;
							arg.push_back(secondArgument);

							firstDoubleWord |= createCodeRegisterIndirect(arg, instructionCode, 0);
						
							string hexCode = longlongToHexString(firstDoubleWord, 4);

							writeSectionContent(hexCode);

							currentSection->incrementLocationCounterBy(4);
						}
						else if (regex_match(secondArgument, REGEX_ADDR_MODE_REG_IND_DISP)) {
							machineCode = (firstDoubleWord << 32) | createCodeRegisterIndirectDisplacement(secondArgument, instructionCode, 0);
								
							string hexCode = longlongToHexString(machineCode, 8);
							
							writeSectionContent(hexCode);
							
							currentSection->incrementLocationCounterBy(8);
						}
						else if (regex_match(secondArgument, REGEX_ADDR_MODE_DOLLAR_PC)) {
							machineCode = (firstDoubleWord << 32) | createCodePCRelative(secondArgument, instructionCode, 0);
							
							string hexCode = longlongToHexString(machineCode, 8);
							
							writeSectionContent(hexCode);
							
							currentSection->incrementLocationCounterBy(8);
						}
					}
				}
				else if (reader->isLoadStoreInstruction(instruction)) {
					cout << "LOAD STORE INSTRUCTION " << word << endl;
					
					string firstArgument = reader->trim(arguments.at(0));

					if (regex_match(firstArgument, REGEX_ADDR_MODE_REG_DIR)) {
						vector<string> arg;
						arg.push_back(firstArgument);

						firstDoubleWord = createCodeRegisterDirect(arg, instructionCode, 0);
					}
					
					string secondArgument = reader->trim(arguments.at(1));
					
					int type = 0;
					
					if (!instruction.compare("LOAD")) {
						type = DOUBLE_WORD;
					}
					else if (!instruction.compare("LOADUB")) {
						type = ZERO_BYTE;
					}
					else if (!instruction.compare("LOADSB")) {
						type = SIGN_BYTE;
					}
					else if (!instruction.compare("LOADUW")) {
						type = ZERO_WORD;
					}
					else if (!instruction.compare("LOADSW")) {
						type = SIGN_WORD;
					}
					else if (!instruction.compare("STORE")) {
						type = DOUBLE_WORD;
					}
					else if (!instruction.compare("STOREB")) {
						type = ZERO_BYTE;
					}
					else if (!instruction.compare("STOREW")) {
						type = ZERO_WORD;
					}
					
					if (!instruction.compare("LOAD")) {
						if (regex_match(secondArgument, REGEX_ADDR_MODE_IMMEDIATE)) {
							machineCode = (firstDoubleWord << 32) | createCodeImmediate(secondArgument, instructionCode, type);
								
							string hexCode = longlongToHexString(machineCode, 8);
							
							writeSectionContent(hexCode);
							
							currentSection->incrementLocationCounterBy(8);
						}
					}
					
					if (regex_match(secondArgument, REGEX_ADDR_MODE_MEM_DIR)) {
						machineCode = (firstDoubleWord << 32) | createCodeMemoryDirect(secondArgument, instructionCode, type);
						
						string hexCode = longlongToHexString(machineCode, 8);
						
						writeSectionContent(hexCode);
						
						currentSection->incrementLocationCounterBy(8);
					}
					else if (regex_match(secondArgument, REGEX_ADDR_MODE_REG_IND)) {
						vector<string> arg;
						arg.push_back(secondArgument);

						firstDoubleWord |= createCodeRegisterIndirect(arg, instructionCode, type);
					
						string hexCode = longlongToHexString(firstDoubleWord, 4);

						writeSectionContent(hexCode);

						currentSection->incrementLocationCounterBy(4);
					}
					else if (regex_match(secondArgument, REGEX_ADDR_MODE_REG_IND_DISP)) {
						machineCode = (firstDoubleWord << 32) | createCodeRegisterIndirectDisplacement(secondArgument, instructionCode, type);
							
						string hexCode = longlongToHexString(machineCode, 8);
						
						writeSectionContent(hexCode);
						
						currentSection->incrementLocationCounterBy(8);
					}
					else if (regex_match(secondArgument, REGEX_ADDR_MODE_DOLLAR_PC)) {
						machineCode = (firstDoubleWord << 32) | createCodePCRelative(secondArgument, instructionCode, type);
						
						string hexCode = longlongToHexString(machineCode, 8);
						
						writeSectionContent(hexCode);
						
						currentSection->incrementLocationCounterBy(8);
					}
				}
				else if (reader->isStackInstruction(instruction)) {
					cout << "STACK INSTRUCTION " << word << endl;
					
					firstDoubleWord = createCodeRegisterDirect(arguments, instructionCode, 0);
					
					string hexCode = longlongToHexString(firstDoubleWord, 4);

					writeSectionContent(hexCode);

					currentSection->incrementLocationCounterBy(4);
				}
				else if (reader->isAritmeticLogicInstruction(instruction)) {
					cout << "ARITM LOGIC INSTRUCTION " << word << endl;
					
					firstDoubleWord = createCodeRegisterDirect(arguments, instructionCode, 0);
					
					string hexCode = longlongToHexString(firstDoubleWord, 4);
					
					writeSectionContent(hexCode);

					currentSection->incrementLocationCounterBy(4);
				}
			}
		}
		else if (reader->isSection(word)) {
			cout << "THIS IS SECTION WITH NAME: " << word << endl;

			currentSection = findSectionByName(word);

			string flags = currentSection->getFlags();

			regex textSection("\\.text(\\.(0|[1-9][0-9]*)){0,1}");
			regex dataSection("\\.data(\\.(0|[1-9][0-9]*)){0,1}");
			regex readOnlySection("\\.rodata(\\.(0|[1-9][0-9]*)){0,1}");
			regex bssSection("\\.bss(\\.(0|[1-9][0-9]*)){0,1}");

			if (regex_match(word, textSection)) {
				flags += "RE";
			}
			else if (regex_match(word, dataSection)) {
				flags += "RW";
			}
			else if (regex_match(word, readOnlySection)) {
				flags += "R";
			}
			else if (regex_match(word, bssSection)) {
				flags += "RW";
			}

			if (previousLineOrg) {
				flags += "F";
				currentSection->setStartAddress(orgAddress);
				previousLineOrg = false;
			}

			currentSection->setFlags(flags);
		}
		else if (reader->isOrgDirective(word)) {
			word = reader->getFirstWord(str);

			orgAddress = stoi(reader->trim(word), nullptr, 0);

			previousLineOrg = true;

		}
		lineCounter++;
	}

	for (int i = 0; i < sectionArray.size(); i++) {
		unsigned int size = sectionArray[i]->getLocationCounter();
		sectionArray[i]->setSectionSize(size);
	}
	
	symbolTable.writeToFile("izlaz.txt");

	for (int i = 0; i < sectionArray.size(); i++) {
		RelocationTable relocationTable = sectionArray[i]->getRelocationTable();
		
		relocationTable.writeToFile(sectionArray[i]->getName(), "izlaz.txt");

		SectionContent sec = sectionArray[i]->getContent();
		sec.writeInFile(sectionArray[i]->getName(), "izlaz.txt");
	}

	ofstream file;
	
	file.open("izlaz.txt", std::ios::app);

	file << "#end" << endl;

	return true;
}