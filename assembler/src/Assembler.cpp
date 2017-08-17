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
		
			for (map<string, int>::iterator it = registerCodes.begin(); it != registerCodes.end(); ++it) {
				cout << it->first << " " << it->second << endl;
			}

		registerCode = registerCodes.at(registerOperand);

		cout << " REG " << registerOperand << " : " << registerCode << endl;

		int a;

		cin >> a;


		machineCode |= (registerCode << (REG0_OFFSET - 5 * i));
	}

	machineCode |= (instructionCode << OPCODE_OFFSET);
	machineCode |= (type << TYPE_OFFSET);
	machineCode |= (addressModeCode << ADDR_MODE_OFFSET);

	cout << "INST" << instructionCode << " ADDR " << addressModeCode << endl;

	return machineCode;
}

bool Assembler::firstPass() {
	if (!isFileExists(inputFileName)) {
		cout << "ERROR: File '" + inputFileName + "' doesn't exists." << endl;
		return false;
	}

	reader = new Reader(inputFileName);

	string str;

	int lineCounter = 1;
	int orderNumberCounter = 1;
	
	while (1) {
		cout << "==================================================" << endl;

		str = reader->readNextLine();

		if (str.empty()) {
			cout << "There is not .end directive."  << endl;
			break;
		}

		str = reader->trim(str);

		cout << str << endl;

		cout << "DISCARDED COMMENT: " << endl;

		str = reader->discardComment(str);

		cout << str << endl;

		cout << "FIRST WORD: " << endl;

		string word = reader->getFirstWord(str);

		bool error = false;

		cout << word << endl;

		if (!word.compare(".end")) {
			cout << "First Pass finised. End of assembler file." << endl;
			break;
		}

		if (reader->isInstruction(word) || reader->isLabel(word)) {
			if (reader->isLabel(word)) {
				string nextWord = reader->getFirstWord(str);

				if (symbolTable.findSymbolByName(word) == nullptr) {
					Symbol symbol;

					symbol.setType("SYM");
					symbol.setOrderNumber(orderNumberCounter++);
					symbol.setName(word.substr(0, word.size() - 1));
					symbol.setSectionNumber(currentSection->getOrderNumber());
					symbol.setSectionOffset(currentSection->getLocationCounter());
					
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
				string instruction = word;

				if (reader->isControlFlowInstruction(instruction)) {
					cout << "CONTROL FLOW INSTRUCTION: " << word << endl;

					if (!instruction.compare("INT")) {
						if (argumentsNumber != 1) {
							cout << "ERROR Line " << lineCounter << ": Unexcepted arguments number. INT excepts 1 argument." << endl;
							break;
						}
					}

					if (!instruction.compare("JMP") || !instruction.compare("CALL")) {
						if (argumentsNumber != 1) {
							cout << "ERROR Line " << lineCounter << ": Unexcepted arguments number. " << instruction << " excepts 1 argument." << endl;
							break;
						}

						for (vector<string>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
							*it = reader->trim(*it);

							cout << *it << endl;

							if (!regex_match(*it, REGEX_ADDR_MODE_MEM_DIR) &&
								!regex_match(*it, REGEX_ADDR_MODE_REG_IND) &&
								!regex_match(*it, REGEX_ADDR_MODE_REG_IND_DISP) &&
								!regex_match(*it, REGEX_ADDR_MODE_DOLLAR_PC)) {

								cout << "ERROR Line " << lineCounter << ": Unexcepted address mode." << endl;
								error = true;
								break;
							}
						}

						if (error) {
							break;
						}
					}

					if (!instruction.compare("RET")) {
						if (argumentsNumber != 0) {
							cout << "ERROR Line " << lineCounter << ": Unexcepted arguments number. " << instruction << " excepts 0 argument." << endl;
							break;
						}
					}

					if (!instruction.compare("JZ")
						|| !instruction.compare("JNZ")
						|| !instruction.compare("JGZ")
						|| !instruction.compare("JGEZ")
						|| !instruction.compare("JLZ")
						|| !instruction.compare("JLEZ")) {

						if (argumentsNumber != 2) {
							cout << "ERROR Line " << lineCounter << ": Unexcepted arguments number. " << instruction << " excepts 2 argument." << endl;
							break;
						}

						if (!regex_match(reader->trim(arguments.at(0)), REGEX_ADDR_MODE_REG_DIR)) {
							cout << "ERROR Line " << lineCounter << ": Unexcepted address mode for first argument." << endl;
							break;
						}

						if (!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_MEM_DIR) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND_DISP) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_DOLLAR_PC)) {

							cout << "ERROR Line " << lineCounter << ": Unexcepted address mode for second argument." << endl;
							break;
						}
					}

					currentSection->incrementLocationCounterBy(CONTROL_FLOW_INST_SIZE);
				}
				else if (reader->isLoadStoreInstruction(instruction)) {
					cout << "LOAD STORE INSTRUCTION" << endl;

					if (argumentsNumber != 2) {
						cout << "ERROR Line " << lineCounter << ": Unexcepted arguments number. " << instruction << " excepts 2 argument." << endl;
						break;
					}

					if (!regex_match(reader->trim(arguments.at(0)), REGEX_ADDR_MODE_REG_DIR)) {
						cout << "ERROR Line " << lineCounter << ": Unexcepted address mode for first argument." << endl;
						break;
					}

					if (!instruction.compare("LOAD") ||
						!instruction.compare("LOADSB") ||
						!instruction.compare("LOADUW") ||
						!instruction.compare("LOADSW")) {

						if (!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_IMMEDIATE) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_DIR) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_MEM_DIR) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND_DISP)) {

							cout << "ERROR Line " << lineCounter << ": Unexcepted address mode for second argument." << endl;
							break;
						}
					}

					if (!instruction.compare("STORE") ||
						!instruction.compare("STOREB") ||
						!instruction.compare("STOREW")) {

						if (regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_IMMEDIATE) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_DIR) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_MEM_DIR) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND) &&
							!regex_match(reader->trim(arguments.at(1)), REGEX_ADDR_MODE_REG_IND_DISP)) {

							cout << "ERROR Line " << lineCounter << ": Unexcepted address mode for second argument." << endl;
							break;
						}
					}

					currentSection->incrementLocationCounterBy(LOAD_STORE_INST_SIZE);
				}
				else if (reader->isStackInstruction(instruction)) {
					if (argumentsNumber != 1) {
						cout << "ERROR Line " << lineCounter << ": Unexcepted arguments number. " << instruction << " excepts 1 argument." << endl;
						break;
					}

					if (!regex_match(reader->trim(arguments.at(0)), REGEX_ADDR_MODE_REG_DIR)) {
						cout << "ERROR Line " << lineCounter << ": Unexcepted address mode for first argument." << endl;
						break;
					}

					currentSection->incrementLocationCounterBy(STACK_INST_SIZE);
				}
				else if (reader->isAritmeticLogicInstruction(instruction)) {
					if (argumentsNumber != 3) {
						cout << "ERROR Line " << lineCounter << ": Unexcepted arguments number. " << instruction << " excepts 3 argument." << endl;
						break;
					}

					for (vector<string>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
						if (!regex_match(reader->trim(*it), REGEX_ADDR_MODE_REG_DIR)) {
							cout << "ERROR Line " << lineCounter << ": Unexcepted address mode for first argument." << endl;
							error = true;
							break;
						}
					}

					if (error) {
						break;
					}

					currentSection->incrementLocationCounterBy(ARITM_LOGIC_INST_SIZE);
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
				
				currentSection = section;
				
				sectionArray.push_back(section);
				
				symbolTable.addSymbol(*section);
			}
		}
		lineCounter++;
	}

	symbolTable.writeToFile("izlaz.txt");
	
	resetSectionCounters();

	for (int i = 0; i < sectionArray.size(); i++) {
		cout << sectionArray[i]->getName() << endl;
	}

	return true;
}

bool Assembler::secondPass() {
	string str;

	int lineCounter = 1;
	
	unsigned long long machineCode = 0;

	unsigned long firstDoubleWord = 0;

	unsigned long long instructionCode = 0;
	unsigned long long addressModeCode = 0;
	unsigned long long firstRegisterCode = 0;
	unsigned long long secondRegisterCode = 0;
	unsigned long long thirdRegisterCode = 0;

	unsigned long secondDoubleWord;

	while (1) {
		cout << "==================================================" << endl;

		str = reader->readNextLine();

		if (str.empty()) {
			cout << "There is not .end directive."  << endl;
			break;
		}

		str = reader->trim(str);
		str = reader->discardComment(str);

		string word = reader->getFirstWord(str);
		
		bool error = false;

		if (!word.compare(".end")) {
			cout << "First Pass finised. End of assembler file." << endl;
			break;
		}

		if (reader->isInstruction(word) || reader->isLabel(word)) {
			if (reader->isLabel(word)) {
				string nextWord = reader->getFirstWord(str);

				if (!nextWord.empty()) {
					if (find(mnemonics.begin(), mnemonics.end(), nextWord) != mnemonics.end()) {
						word = nextWord;
					}
					else if (find(dataDefining.begin(), dataDefining.end(), nextWord) != dataDefining.end()) {
						cout << "THIS IS LABEL WITH DATA DEFINITION " << nextWord << endl;
						
						vector<string> dataDefineFields = reader->split(str, ',');
						
						for (vector<string>::iterator it = dataDefineFields.begin(); it != dataDefineFields.end(); ++it) {
							*it = reader->trim(*it);
							
							smatch match;
							
							if (it->find_first_of(" ") != string::npos) {
								vector<string> dupArguments = reader->split(*it, ' ');
								
								string firstArgument = dupArguments.at(0);
								string secondArgument = dupArguments.at(1);
								string thirdArgument = dupArguments.at(2);
								
								/*
									UPISI U TRENUTNU SEKCIJU SADRZAJ KOJI GENERISU DD, DW i DB
								*/
								
								cout << firstArgument << " " << secondArgument << " " << thirdArgument << endl;  
							}
								
							const string str = *it;
							string registerOperand = "";

							if (regex_search(str.begin(), str.end(), match, REGEX_CONST_EXPRESSION)) {
								registerOperand = match[1];
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
					
					if (!instruction.compare("JMP") || !instruction.compare("CALL")) {

						for (vector<string>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
							*it = reader->trim(*it);

							if (regex_match(*it, REGEX_ADDR_MODE_MEM_DIR)) {

							}
							else if (regex_match(*it, REGEX_ADDR_MODE_REG_IND)) {
								regex rgx("\\[((R[0-9]{1}|1[0-5])|PC|SP){1}\\]");
								smatch match;
								
								const string str = *it;
								string registerOperand = "";

								if (regex_search(str.begin(), str.end(), match, rgx))
									registerOperand = match[1];

								firstRegisterCode = registerCodes[registerOperand];
								addressModeCode = REG_IND_ADDR_MODE;
								
								firstDoubleWord |= (instructionCode << OPCODE_OFFSET);
								firstDoubleWord |= (addressModeCode << ADDR_MODE_OFFSET);
								firstDoubleWord |= (firstRegisterCode << REG0_OFFSET);
								
								/*
									WRITE IN CURRENT SECTION CONTENT
								*/
							}
							else if (regex_match(*it, REGEX_ADDR_MODE_REG_IND_DISP)) {
								regex rgx("\\[((R[0-9]{1}|1[0-5])|PC|SP){1}(\\s)*\\+(\\s)*(.*)\\]");
								smatch match;

								const string str = *it;
								string registerOperand = "";
								string displacement = "";

								if (regex_search(str.begin(), str.end(), match, rgx)) {
									registerOperand = match[1];
									displacement = match[5];
								}
								
								cout << "REG" << registerOperand << " " << displacement << endl;
								
								string infix;
								vector<string> postfix;
								string symbol;

								const string stringDisplacement = displacement;
								string operation;
																
								if (regex_search(stringDisplacement.begin(), stringDisplacement.end(), match, REGEX_CONST_EXPRESSION)) {
									symbol = match [1];
									operation = match[3]; 
									infix = match [4];
								}

								cout << "DISPLACEMENT: " << symbol << " " << infix << endl;
								
								SymbolTableEntry* entry = symbolTable.findSymbolByName(symbol);
								unsigned int sectionNumber = 0;
								unsigned int address = 0;
																
								if (entry != nullptr) {
									sectionNumber = entry->getSectionNumber();	
									Section* section = findSectionByOrdNumber(sectionNumber);
									address = section->getStartAddress();
								}

								postfix = infixToPostfixExpression(infix);

								int result = evaluateExpression(postfix);
								
								cout << "RESULT" << result << endl;
								
								firstRegisterCode = registerCodes[registerOperand];
								addressModeCode = REG_IND_DISP_ADDR_MODE;
								
								firstDoubleWord |= (instructionCode << OPCODE_OFFSET);
								firstDoubleWord |= (addressModeCode << ADDR_MODE_OFFSET);
								firstDoubleWord |= (firstRegisterCode << REG0_OFFSET);
								
								if (operation.compare("+")) {
									secondDoubleWord = address + result;
								} else {
									secondDoubleWord = address - result;
								}
								
							}
							else if (regex_match(*it, REGEX_ADDR_MODE_DOLLAR_PC)) {
								regex rgx("\\$([a-z|A-Z|_][a-zA-Z0-9_]+|0x[0-9]{0,8})");
								smatch match;
								
								regex rgxAddress("(0x[0-9]{0,8})");
								smatch matchAddress;
								
								const string str = *it;
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
								
								if (symbol.compare("")) {
									SymbolTableEntry* entry = symbolTable.findSymbolByName(symbol);
									unsigned int sectionNumber = 0;
									
									if (entry != nullptr) {
										sectionNumber = entry->getSectionNumber();	
										Section* section = findSectionByOrdNumber(sectionNumber);
										
										address = section->getStartAddress();
									}
								}
								
								firstRegisterCode == registerCodes["PC"];
								addressModeCode = REG_IND_DISP_ADDR_MODE;
								
								firstDoubleWord |= (instructionCode << OPCODE_OFFSET);
								firstDoubleWord |= (addressModeCode << ADDR_MODE_OFFSET);
								firstDoubleWord |= (firstRegisterCode << REG0_OFFSET);
								
								secondDoubleWord = address;
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
						
							string hexCode = longlongToHexString(firstDoubleWord, 4);

							writeSectionContent(hexCode);

							currentSection->incrementLocationCounterBy(4);
						}

						string secondArgument = reader->trim(arguments.at(1));

						if (regex_match(secondArgument, REGEX_ADDR_MODE_MEM_DIR)) {

						}
						else if (regex_match(secondArgument, REGEX_ADDR_MODE_REG_IND)) {
							regex rgx("\\[((R[0-9]{1}|1[0-5])|PC|SP){1}\\]");
							smatch match;
							
							const string str = secondArgument;
							string registerOperand = "";

							if (regex_search(str.begin(), str.end(), match, rgx))
								registerOperand = match[1];

							secondRegisterCode = registerCodes[registerOperand];
							addressModeCode = REG_IND_ADDR_MODE;
							
							firstDoubleWord |= (instructionCode << OPCODE_OFFSET);
							firstDoubleWord |= (addressModeCode << ADDR_MODE_OFFSET);
							firstDoubleWord |= (firstRegisterCode << REG1_OFFSET);
						}
						else if (regex_match(secondArgument, REGEX_ADDR_MODE_REG_IND_DISP)) {
							regex rgx("\\[((R[0-9]{1}|1[0-5])|PC|SP){1}(\\s)*\\+(\\s)*(.*)\\]");
							smatch match;

							const string str = secondArgument;
							string registerOperand = "";
							string displacement = "";

							if (regex_search(str.begin(), str.end(), match, rgx)) {
								registerOperand = match[1];
								displacement = match[5];
							}
							
							cout << "REG" << registerOperand << " " << displacement << endl;
							
							string infix;
							vector<string> postfix;
							string symbol;

							const string stringDisplacement = displacement;
							string operation;
															
							if (regex_search(stringDisplacement.begin(), stringDisplacement.end(), match, REGEX_CONST_EXPRESSION)) {
								symbol = match [1];
								operation = match[3]; 
								infix = match [4];
							}

							cout << "DISPLACEMENT: " << symbol << " " << infix << endl;
							
							SymbolTableEntry* entry = symbolTable.findSymbolByName(symbol);
							unsigned int sectionNumber = 0;
							unsigned int address = 0;
							
							if (entry != nullptr) {
								sectionNumber = entry->getSectionNumber();	
								Section* section = findSectionByOrdNumber(sectionNumber);
								
								address = section->getStartAddress();
							}

							postfix = infixToPostfixExpression(infix);

							int result = evaluateExpression(postfix);
							
							cout << "RESULT" << result << endl;
							
							firstRegisterCode = registerCodes[registerOperand];
							addressModeCode = REG_IND_DISP_ADDR_MODE;
							
							firstDoubleWord |= (instructionCode << OPCODE_OFFSET);
							firstDoubleWord |= (addressModeCode << ADDR_MODE_OFFSET);
							firstDoubleWord |= (firstRegisterCode << REG0_OFFSET);
							
							if (operation.compare("+")) {
								secondDoubleWord = address + result;
							} else {
								secondDoubleWord = address - result;
							}
						}
						else if (regex_match(secondArgument, REGEX_ADDR_MODE_DOLLAR_PC)) {
							regex rgx("\\$([a-z|A-Z|_][a-zA-Z0-9_]+|0x[0-9]{0,8})");
							smatch match;
							
							regex rgxAddress("(0x[0-9]{0,8})");
							smatch matchAddress;
							
							const string str = secondArgument;
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
							
							if (symbol.compare("")) {
								SymbolTableEntry* entry = symbolTable.findSymbolByName(symbol);
								unsigned int sectionNumber = 0;
								
								if (entry != nullptr) {
									sectionNumber = entry->getSectionNumber();	
									Section* section = findSectionByOrdNumber(sectionNumber);
									
									address = section->getStartAddress();
								}
							}
							
							firstRegisterCode == registerCodes["PC"];
							addressModeCode = REG_IND_DISP_ADDR_MODE;
							
							firstDoubleWord |= (instructionCode << OPCODE_OFFSET);
							firstDoubleWord |= (addressModeCode << ADDR_MODE_OFFSET);
							firstDoubleWord |= (firstRegisterCode << REG0_OFFSET);
							
							secondDoubleWord = address;
						}
					}

					currentSection->incrementLocationCounterBy(CONTROL_FLOW_INST_SIZE);
				}
				else if (reader->isLoadStoreInstruction(instruction)) {
					cout << "LOAD STORE INSTRUCTION " << word << endl;

					currentSection->incrementLocationCounterBy(LOAD_STORE_INST_SIZE);
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
					
					addressModeCode = REG_DIR_ADDR_MODE;
					
					regex rgx("((R[0-9]{1}|1[0-5])|PC|SP){1}");
					smatch match;
					
					const string firstArgument = arguments.at(0);
					string registerOperand = "";

					if (regex_search(firstArgument.begin(), firstArgument.end(), match, rgx))
						registerOperand = match[1];

					firstRegisterCode = registerCodes[registerOperand];

					const string secondArgument = arguments.at(1);
				
					if (regex_search(firstArgument.begin(), firstArgument.end(), match, rgx))
						registerOperand = match[1];

					firstRegisterCode = registerCodes[registerOperand];

					const string thirdArgument = arguments.at(2);
					
					if (regex_search(firstArgument.begin(), firstArgument.end(), match, rgx))
						registerOperand = match[1];

					firstRegisterCode = registerCodes[registerOperand];

					currentSection->incrementLocationCounterBy(ARITM_LOGIC_INST_SIZE);
				}
			}
		}
		else if (reader->isSection(word)) {
			cout << "THIS IS SECTION WITH NAME: " << word << endl;

			currentSection = findSectionByName(word);

			cout << currentSection->getName() << endl;
		}
		lineCounter++;
	}

	symbolTable.writeToFile("izlaz.txt");

	for (int i = 0; i < sectionArray.size(); i++) {
		SectionContent sec = sectionArray[i]->getContent();
		sec.writeInFile(sectionArray[i]->getName(), "izlaz.txt");
	}

	return true;
}