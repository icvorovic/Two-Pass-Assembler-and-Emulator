#include "Emulator.h"
#include "Constants.h"
#include "Symbol.h"
#include "Section.h"
#include "Utility.h"

#include <iostream>
#include <sstream>

Emulator::Emulator(string inputFile) {
    reader = new Reader(inputFile);
}

Emulator::~Emulator() {
    delete reader;
    delete symbolTable;
    delete currentSection;

    sectionArray.clear();
}

unsigned long Emulator::getInstuctionCode() {
    return (doubleWord >> OPCODE_OFFSET) & OPCODE_MASK;
}
unsigned long Emulator::getAddressModeCode() {
    return (doubleWord >> ADDR_MODE_OFFSET) & ADDR_MODE_MASK;
}

unsigned long Emulator::getR0Code() {
    return (doubleWord >> REG0_OFFSET) & REG_MASK;
}

unsigned long Emulator::getR1Code() {
    return (doubleWord >> REG1_OFFSET) & REG_MASK;
}

unsigned long Emulator::getR2Code() {
    return (doubleWord >> REG2_OFFSET) & REG_MASK;
}

unsigned long Emulator::getTypeCode() {
    return (doubleWord >> TYPE_OFFSET) & TYPE_MASK;
}

void Emulator::updateSectionSymbolValues(Section *section) {
    unsigned int orderNumber = section->getOrderNumber();
    unsigned int sectionStartAddress = section->getStartAddress();

    string flags = section->getFlags();
    
    if (flags.find_first_of("F") == string::npos) {
        vector<Symbol*> sectionSymbols = symbolTable->getSectionSymbols(section);

        for (vector<Symbol*>::iterator it = sectionSymbols.begin(); it != sectionSymbols.end(); ++it) {
            unsigned long symbolValue = (*it)->getValue();
            (*it)->setValue(symbolValue + sectionStartAddress);
        }
    }
}

string Emulator::getErrorDescription() {
    return errorDescription;
}

long Emulator::readDoubleWord() {
    long result = 0;

    for (int i = 0; i < 4; i++) {
        result |= (MEMORY[REGISTER[PC]++] << (8 * i));
    }

    return result;
}

void Emulator::push(unsigned int R0Index) {

    for (int i = 0; i < 4); i++) {
        MEMORY[++REGISTER[SP]] = (char)((REGISTER[R0Index] >> (8 * i)) && 0xFF);
    }
}

long Emulator::pop() {
    long result = 0;

    for (int i = 0; i < 4; i++) {
        result |= (MEMORY[REGISTER[SP]--] << (8 * i));
    }

    return result;
}

bool Emulator::isSectionsIntersect() {
    error = false;

    for (vector<Section*>::iterator it = sectionArray.begin(); it != sectionArray.end(); ++it) {
        string flags = (*it)->getFlags();

        unsigned long sectionSize = (*it)->getSectionSize();
        unsigned long startAddress = (*it)->getStartAddress();

        if (flags.find_first_of("F") != string::npos) {
            for (vector<Section*>::iterator it2 = sectionArray.begin(); it2 != sectionArray.end(); ++it2) {
                if ((*it) != (*it2)) {
                    unsigned long secondSectionSize = (*it2)->getSectionSize();
                    unsigned long secondStartAddress = (*it2)->getStartAddress();
                
                    if (((startAddress >= secondStartAddress) && (startAddress <= (secondStartAddress + secondSectionSize))) 
                      || ((secondStartAddress >=  startAddress) && (secondStartAddress <= (startAddress + sectionSize)))) {
                        error = true;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool Emulator::readInputStructures() {
    string line = reader->readNextLine();

    line = reader->trim(line);

    string relocationTablePrefix = "#rel";
    string sectionContentPrefix = "<";

    bool isRelocationTable = false;
    bool isSectionContent = false;

    if (!line.compare("#TabelaSimbola")) {
        symbolTable = new SymbolTable();

        while (true) {
            line = reader->readNextLine();
            line = reader->trim(line);

            if (line.substr(0, relocationTablePrefix.size()) == relocationTablePrefix
                ||line.substr(0, sectionContentPrefix.size()) == sectionContentPrefix) {
                break;
            }

            vector<string> arguments = reader->split(line, ' ');

            if (arguments.at(0) == "SYM") {
                Symbol *symbol = new Symbol();

                symbol->setType("SYM");
                symbol->setOrderNumber(stoi(arguments.at(1), nullptr, 0));
                symbol->setName(arguments.at(2));
                symbol->setSectionNumber(stoi(arguments.at(3), nullptr, 0));
                
                symbol->setValue(stoi(arguments.at(4), nullptr, 0));
                symbol->setFlag((arguments.at(5)).at(0));
                
                symbolTable->addSymbol(symbol);

                cout << "SYM" << endl;
            } 

            if (arguments.at(0) == "SEG"){
                Section *section = new Section();

                section->setType("SEG");
                section->setOrderNumber(stoi(arguments.at(1), nullptr, 0));
                section->setName(arguments.at(2));
                section->setSectionNumber(stoi(arguments.at(3), nullptr, 0));

                section->setStartAddress(stoi(arguments.at(4), nullptr, 0));
                section->setSectionSize(stoi(arguments.at(5), nullptr, 0));
                section->setFlags(arguments.at(6));

                symbolTable->addSymbol(section);
                sectionArray.push_back(section);
                                
                cout << "SEG" << endl;
            }
         }
    }

    symbolTable->writeToFile("izlaz.txt");

    while (line.compare("#end")) {
        cout << "LINE : " << line << endl;

        if (line.substr(0, relocationTablePrefix.size()) == relocationTablePrefix) {
            string sectionName = line.substr(relocationTablePrefix.size(), line.size() - relocationTablePrefix.size());
            
            currentSection = (Section*)symbolTable->findSymbolByName(sectionName);

            line = reader->readNextLine();
            
            line = reader->trim(line);

            isRelocationTable = true;
            isSectionContent = false;

            cout << "REL" <<  sectionName << endl;
        }
        else if (line.substr(0, sectionContentPrefix.size()) == sectionContentPrefix) {
            string sectionName = line.substr(1, line.size() - 2);
            
            currentSection = (Section*)symbolTable->findSymbolByName(sectionName);
            
            line = reader->readNextLine();
            
            line = reader->trim(line);

            isRelocationTable = false;
            isSectionContent = true;

            cout << "SECTION" << sectionName << endl;
        }

        if (isRelocationTable) {
            RelocationTable relocationTable = currentSection->getRelocationTable();

            RelocationTableEntry relocationRecord;

            vector<string> arguments = reader->split(line, ' ');

            relocationRecord.setAddress(stoi(arguments.at(0), nullptr, 0));
            relocationRecord.setType((arguments.at(1)).at(0));
            relocationRecord.setOrderNumber(stoi(arguments.at(2), nullptr, 0));

            relocationTable.addRelocationRecord(relocationRecord);

            currentSection->setRelocationTable(relocationTable);
        }

        if (isSectionContent) {
            SectionContent content = currentSection->getContent();

            for (int i = 0; i < line.length(); i++) {
                if (line.at(i) == ' ') {
                    continue;
                }

                content.addByte(line.at(i));
            }

            currentSection->setContent(content);
        }

        line = reader->readNextLine();
        
        line = reader->trim(line);
    }
    
    for (int i = 0; i < sectionArray.size(); i++) {
		RelocationTable relocationTable = sectionArray[i]->getRelocationTable();
		
		relocationTable.writeToFile(sectionArray[i]->getName(), "izlaz.txt");

		SectionContent sec = sectionArray[i]->getContent();
		sec.writeInFile(sectionArray[i]->getName(), "izlaz.txt");
	}

    return true;
}

bool Emulator::fillMemory() {
    if (isSectionsIntersect()) {
        errorDescription = "Sections cannot be located in memory. There is intersecion.";
        error = true;
        return false;
    }

    unsigned long startAddress = 0x100;
    unsigned long fillStartAddress = 0x0;

    bool updateStartFillAddress = false;

    for(vector<Section*>::iterator it = sectionArray.begin(); it != sectionArray.end(); ++it) {
        string flags = (*it)->getFlags();

        SectionContent content = (*it)->getContent();
        vector<char> byteContent = content.getByteContent();

        if (flags.find_first_of("F") != string::npos) {
            fillStartAddress = (*it)->getStartAddress();
        } else {
            (*it)->setStartAddress(startAddress);
            
            updateSectionSymbolValues((*it));

            fillStartAddress = startAddress;
            updateStartFillAddress = true;
        }

        int i = 0;

        while (i < byteContent.size()) {
            stringstream ss;

            string firstChar;
            string secondChar;

            ss << byteContent[i];
            ss >> firstChar;

            ss.clear();
        
            ss << byteContent[i + 1];
            ss >> secondChar;

            string byteRepresentation = "0x" + firstChar + secondChar;
            
            unsigned char value = stoi(byteRepresentation, nullptr, 0);

            MEMORY[fillStartAddress++] = value;

            i += 2;
        }

        if (updateStartFillAddress) {
            startAddress = fillStartAddress;
            updateStartFillAddress = false;
        }
    }

    symbolTable->writeToFile("izlaz.txt");
    

    for (int i = 0; i < sectionArray.size(); i++) {
		RelocationTable relocationTable = sectionArray[i]->getRelocationTable();
		
		relocationTable.writeToFile(sectionArray[i]->getName(), "izlaz.txt");

		SectionContent sec = sectionArray[i]->getContent();
		sec.writeInFile(sectionArray[i]->getName(), "izlaz.txt");
	}

    return true;
}

bool Emulator::execute() {
    Symbol* startSymbol = (Symbol*) symbolTable->findSymbolByName("_start");

    if (startSymbol == nullptr) {
        error = true;
        errorDescription = "Symbol \e[1m _start \e[0m is not defined.";
        return false;
    }

    REGISTER[PC] = startSymbol->getValue();

    doubleWord = readDoubleWord();

    unsigned long instruction = getInstuctionCode();
    unsigned long addressMode = getAddressModeCode();
    unsigned long R0Index = getR0Code();
    unsigned long R1Index = getR1Code();
    unsigned long R2Index = getR2Code();
    unsigned long type = getTypeCode();

    switch (instruction) {
        

        //  Stack instruction
        case instructions["PUSH"]:
            cout << "PUSH" << endl;
            
            push(R0Index);
            
            break;
        case instruction["POP"]:
            cout << "POP" << endl;

            REGISTER[R0Index] = pop();

            break;
        //  Aritmetic and logic instructions
        case instructions["ADD"]:
            cout << "ADD" << endl;   
            REGISTER[R0Index] = REGISTER[R1Index] + REGISTER[R2Index];
            break;
        case instructions["SUB"]:
            cout << "SUB" << endl;   
            REGISTER[R0Index] = REGISTER[R1Index] - REGISTER[R2Index];
            break;
        case instructions["MUL"]:
            cout << "MUL" << endl;   
            REGISTER[R0Index] = REGISTER[R1Index] * REGISTER[R2Index];
            break;
        case instructions["DIV"]:
            cout << "DIV" << endl;
            REGISTER[R0Index] = REGISTER[R1Index] / REGISTER[R2Index];
            break;
        case instructions["MOD"]:
            cout << "MOD" << endl;
            REGISTER[R0Index] = REGISTER[R1Index] % REGISTER[R2Index];
            break;
        case instructions["AND"]:
            cout << "ADD" << endl;
            REGISTER[R0Index] = REGISTER[R1Index] & REGISTER[R2Index];
            break;
        case instructions["OR"]:
            cout << "OR" << endl;
            REGISTER[R0Index] = REGISTER[R1Index] | REGISTER[R2Index];
            break;
        case instructions["XOR"]:
            cout << "XOR" << endl;
            REGISTER[R0Index] = REGISTER[R1Index] ^ REGISTER[R2Index];
            break;
        case instructions["NOT"]:
            cout << "NOT" << endl;
            REGISTER[R0Index] = !REGISTER[R1Index];
            break;
        case instructions["ASL"]:
            cout << "ASL" <<endl;
            REGISTER[R0Index] = REGISTER[R1Index] << REGISTER[R2Index];
            break;
        case instructions["ASR"]:
            cout << "ASR" << endl;
            REGISTER[R0Index] = REGISTER[R1Index] >> REGISTER[R2Index];
            break;
    }


    return true;
}