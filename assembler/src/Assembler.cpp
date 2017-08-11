#include "Assembler.h"
#include <string>
#include <iostream>
#include <regex>
#include "Reader.h"
#include "Constants.h"
#include "Utility.h"

Assembler::Assembler(string inputFileName, string outputFileName) {
	this->inputFileName = inputFileName;
	this->outputFileName = outputFileName;	
}

Assembler::~Assembler() {
	relocationTableArray.clear();
	sectionContentArray.clear();
}

void Assembler::resetSectionCounters() {
	for (vector<SectionContent>::iterator it = sectionContentArray.begin(); it != sectionContentArray.end(); ++it) {
		Section section = it->getSection();	
		section.resetLocationCounter();
	}
}

bool Assembler::firstPass() {
	if (!isFileExists(inputFileName)) {
		cout << "ERROR: File '" + inputFileName + "' doesn't exists." << endl;
		return false;
	}

	Reader *reader = new Reader(inputFileName);

	string str;

	int lineCounter = 1;

	Section currentSection;
	
	while (1) {
		cout << "==================================================" << endl;

		str = reader->readNextLine();

		if (str.empty()) break;

		str = reader->trim(str);

		cout << str << endl;

		cout << "DISCARDED COMMENT: " << endl;

		str = reader->discardComment(str);

		cout << str << endl;

		cout << "FIRST WORD: " << endl;

		string word = reader->getFirstWord(str);

		bool error = false;

		cout << word << endl;

		if (reader->isInstruction(word) || reader->isLabel(word)) {
			if (reader->isLabel(word)) {
				string nextWord = reader->getFirstWord(str);

				if (symbolTable.findSymbolByName(word) == nullptr) {
					Symbol symbol;

					symbol.setType("SYM");
					symbol.setName(word.substr(0, word.size() - 1));
					symbol.setSectionNumber(currentSection.getOrderNumber());
					symbol.setSectionOffset(currentSection.getLocationCounter());

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
				cout << "THIS IS INSTRUCTION" << endl;

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

					currentSection.incrementLocationCounterBy(CONTROL_FLOW_INST_SIZE);
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

					currentSection.incrementLocationCounterBy(LOAD_STORE_INST_SIZE);
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

					currentSection.incrementLocationCounterBy(STACK_INST_SIZE);
				}
				else if (reader->isAritmeticLogicInstruction(instruction)) {
					if (argumentsNumber != 3) {
						cout << "ERROR Line " << lineCounter << ": Unexcepted arguments number. " << instruction << " excepts 3 argument." << endl;
						break;
					}

					for (vector<string>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
						if (!regex_match(reader->trim(*it), REGEX_ADDR_MODE_REG_DIR)) {
							cout << "ERROR Line " << lineCounter << ": Unexcepted address mode for first argument." << endl;
							break;
						}
					}

					currentSection.incrementLocationCounterBy(ARITM_LOGIC_INST_SIZE);
				}
			}

			
		}
		else if (reader->isSection(word)) {
			cout << "THIS IS SECTION WITH NAME: " << word << endl;

			if (symbolTable.findSymbolByName(word) == nullptr) {
				Section section;

				currentSection = section;

				section.setType("SEG");
				section.setName(word);
				section.setSectionNumber(currentSection.getOrderNumber());
				section.setSectionOffset(currentSection.getLocationCounter());

				symbolTable.addSymbol(section);
			}
		}
		lineCounter++;
	}

	symbolTable.writeToFile("izlaz.txt");

	return true;
}

bool secondPass() {
	
	return true;
}