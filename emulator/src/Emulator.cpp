#include "Emulator.h"
#include "Constants.h"
#include "Symbol.h"
#include "Section.h"
#include "Utility.h"
#include "RelocationTableEntry.h"

#include <iostream>
#include <sstream>
#include <list>

bool  Emulator::finishExecution = true;

Emulator::Emulator(string inputFile) {
    reader = new Reader(inputFile);

    REGISTER[SP] = 0x4000;
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

    cout << "UPDATE SECITON: " << section->getName() << endl;
    
    if (flags.find_first_of("F") == string::npos) {
        vector<Symbol*> sectionSymbols = symbolTable->getSectionSymbols(section);

        for (vector<Symbol*>::iterator it = sectionSymbols.begin(); it != sectionSymbols.end(); ++it) {
            unsigned long symbolValue = (*it)->getValue();
            (*it)->setValue(symbolValue + sectionStartAddress);

            cout << "UPDATE SYMBOL: " << (*it)->getName() << " VALUE:" << (*it)->getValue() << endl; 
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

long Emulator::read(unsigned int address, unsigned int type) {
    switch(type) {
        case DOUBLE_WORD:
            return readDoubleWord(address);
        case ZERO_WORD:
            return readUnsignedWord(address);
        case SIGN_WORD:
            return readSignedWord(address);
        case ZERO_BYTE:
            return readUnsignedByte(address);
        case SIGN_BYTE:
            return readSignedByte(address);
    }
}

long Emulator::readDoubleWord(unsigned int address) {
    long result = 0;

    for (int i = 0; i < 4; i++) {
        result |= (MEMORY[address++] << (8 * i));
    }

    return result;
}

long Emulator::readUnsignedWord(unsigned int address) {
    long result = 0;
    
    for (int i = 0; i < 2; i++) {
        result |= (MEMORY[address++] << (8 * i));
    }

    return result;
}

long Emulator::readSignedWord(unsigned int address) {
    long result = 0;
    
    for (int i = 0; i < 2; i++) {
        result |= (MEMORY[address++] << (8 * i));
    }

    int signBit = (result & (1 << 15)) >> 15;

    if (signBit == 1) {
        result |= 0xFFFF0000;
    }

    return result;
}

long Emulator::readUnsignedByte(unsigned int address) {
    long result = 0;
    
    result |= MEMORY[address];

    return result;
}

long Emulator::readSignedByte(unsigned int address) {
    long result = 0;
    
    result |= MEMORY[address];

    int signBit = (result & (1 << 7)) >> 7;
    
    if (signBit == 1) {
        result |= 0xFFFFFF00;
    }

    return result;
}

void Emulator::write(const int doubleWordParam, unsigned int address, unsigned int type) {
    switch(type) {
        case DOUBLE_WORD:
            writeDoubleWord(doubleWordParam, address);
            break;
        case ZERO_WORD:
            writeWord(doubleWordParam, address);
            break;
        case ZERO_BYTE:
            writeByte(doubleWordParam, address);
            break;
    }
}

void Emulator::writeDoubleWord(const int doubleWordParam, unsigned int address) {
    cout << "ADDR=" << std::hex << address << " VALUE=" << std::hex << (int)(doubleWordParam) << endl;

    for (int i = 0; i < 4; i++) {
        cout << "WRITE MEMORY ADDRESS=0x" << std::hex << address << " VALUE=" << std::hex << (int)((doubleWordParam >> (8 * i)) & 0xFF) << endl;
        MEMORY[address++] = (char)((doubleWordParam >> (8 * i)) & 0xFF);
    }
}

void Emulator::writeWord(const int doubleWordParam, unsigned int address) {
    for (int i = 0; i < 2; i++) {
        MEMORY[address++] = (char)((doubleWordParam >> (8 * i)) & 0xFF);
    }
}

void Emulator::writeByte(const int doubleWordParam, unsigned int address) {
    MEMORY[address] = (char)(doubleWordParam & 0xFF);
}

void Emulator::push(unsigned int R0Index) {
    for (int i = 0; i < 4; i++) {
        MEMORY[++REGISTER[SP]] = (char)((REGISTER[R0Index] >> (8 * i)) & 0xFF);
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

            cout << "FILL ADDRESS=0x" << std::hex << fillStartAddress << " WITH VALUE=0x" << std::hex << (int)value << endl;

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

void Emulator::correctRelocationMemory() {
    for (int i = 0; i < sectionArray.size(); i++) {
		RelocationTable relocationTable = sectionArray[i]->getRelocationTable();
        unsigned int startSectionAddress = sectionArray[i]->getStartAddress();
        
        string flags = sectionArray[i]->getFlags();

        list<RelocationTableEntry> relocationList = relocationTable.getRelocationList();

        for (list<RelocationTableEntry>::iterator it = relocationList.begin(); it != relocationList.end(); ++it) {
            char type = it->getType();
            unsigned int orderNumber = it->getOrderNumber();
            unsigned int address = it->getAddress();

            Section *section = (Section*) symbolTable->findSymbolByOrderNumber(orderNumber);
            unsigned int sectionAddress = section->getStartAddress();

            if (section != nullptr) {
                if (flags.find_first_of("F") == string::npos) {
                    address += startSectionAddress;
                }
                
                long oldValue = readDoubleWord(address);

                long newValue = oldValue + sectionAddress;

                cout << "ADDRESS=0x" << std::hex << address << " VALUE=0x" << std::hex << (int)(newValue) << endl;               

                writeDoubleWord(newValue, address);

                cout << "OLD_VALUE=0x" << std::hex << oldValue << " NEW_VALUE=0x" << std::hex << newValue << endl; 

                cout << "CORRECT ADDRESS=0x" << std::hex << address << " WITH VALUE=0x" << std::hex << (int)(newValue & 0xFF) << endl;
                cout << "CORRECT ADDRESS=0x" << std::hex << address + 1 << " WITH VALUE=0x" << std::hex << (int)((newValue >> 8) & 0xFF) << endl;
                cout << "CORRECT ADDRESS=0x" << std::hex << address + 2 << " WITH VALUE=0x" << std::hex << (int)((newValue >> 16) & 0xFF) << endl;
                cout << "CORRECT ADDRESS=0x" << std::hex << address + 3 << " WITH VALUE=0x" << std::hex << (int)((newValue >> 24) & 0xFF) << endl;
               
                
            }
        }
	}
}

bool Emulator::execute() {
    Symbol* startSymbol = (Symbol*) symbolTable->findSymbolByName("_start");

    Emulator::finishExecution = false;

    if (startSymbol == nullptr) {
        error = true;
        errorDescription = "Symbol \e[1m _start \e[0m is not defined.";
        Emulator::finishExecution = true;
        return false;
    }

    REGISTER[PC] = startSymbol->getValue();

    cout << "EXECUTE STARTED... " << endl;

    while (true) {

        doubleWord = readDoubleWord();

        unsigned long instruction = getInstuctionCode();
        unsigned long addressMode = getAddressModeCode();
        unsigned long R0Index = getR0Code();
        unsigned long R1Index = getR1Code();
        unsigned long R2Index = getR2Code();
        unsigned long type = getTypeCode();

        unsigned long address = 0;

        string instructionString = "";

        for (map<string, unsigned long>::iterator it = instructions.begin(); it != instructions.end(); ++it) {
            if (it->second == instruction) {
                instructionString = it->first;
                break;
            }
        }

        cout << "0x" << std::hex << doubleWord << endl;
        cout << "0x" << std::hex << instruction << endl;

        //  Control flow instructions

        if ((instruction >= instructions["INT"]) && (instruction <= instructions["JLEZ"])) {
            if ((instruction >= instructions["INT"]) && (instruction <= instructions["CALL"])) {
                if (instruction == instructions["INT"]) {
                    /*
                        BROJ PREKIDA JE U REGISTRU R0
                    */
                    unsigned int interruptNumber = REGISTER[0];

                    if (interruptNumber == 0) {
                        cout << "EXECUTION FINISHED. " << endl;
                        break;
                    }
                }
                else if (instruction == instructions["JMP"] || instruction == instructions["CALL"]) {
                    if (instruction == instructions["CALL"]) {
                        push(PC);
                    }

                    if (addressMode == MEM_DIR_ADDR_MODE) {
                        address = readDoubleWord();
                        REGISTER[PC] = address;
                    }

                    if (addressMode == REG_IND_ADDR_MODE) {
                        address = REGISTER[R0Index];
                        REGISTER[PC] = address;
                    }

                    if (addressMode == REG_IND_DISP_ADDR_MODE) {
                        if (R0Index != PC) {
                            address = REGISTER[R0Index] + readDoubleWord();
                            REGISTER[PC] = address;
                        } else {
                            REGISTER[PC] = REGISTER[PC] + readDoubleWord() - 8;
                        }
                    }
                }
                else if (instruction == instructions["RET"]) {
                    REGISTER[PC] = pop();
                }
            }

            if ((instruction >= instructions["JZ"]) && (instruction <= instructions["JLEZ"])) {
                unsigned int registerContent = REGISTER[R0Index];

                if (addressMode == MEM_DIR_ADDR_MODE) {
                    address = readDoubleWord();
                }

                if (addressMode == REG_IND_ADDR_MODE) {
                    address = REGISTER[R0Index];
                }

                if (addressMode == REG_IND_DISP_ADDR_MODE) {
                    if (R0Index != PC) {
                        address = REGISTER[R0Index] + readDoubleWord();
                    } else {
                        address = REGISTER[PC] + readDoubleWord() - 8;
                    }
                }

                if (instruction == instructions["JZ"] && registerContent == 0) {
                    REGISTER[PC] = address;
                }
                else if (instruction == instructions["JNZ"] && registerContent != 0) {
                    REGISTER[PC] = address;
                }
                else if (instruction == instructions["JGZ"] && registerContent > 0) {
                    REGISTER[PC] = address;
                }
                else if (instruction == instructions["JGEZ"] && registerContent >= 0) {
                    REGISTER[PC] = address;
                }   
                else if (instruction == instructions["JLZ"] && registerContent < 0) {
                    REGISTER[PC] = address;
                }
                else if (instruction == instructions["JLEZ"] && registerContent <= 0) {
                    REGISTER[PC] = address;
                }
            }   
        }

        //  Load/Store instructions
        if ((instruction >= instructions["LOAD"]) && (instruction <= instructions["STORE"])) {
            string coutString = instructionString + " R" + to_string(R0Index) + ", ";

            unsigned int address;

            if ((addressMode == REG_IND_DISP_ADDR_MODE) ||
                addressMode == MEM_DIR_ADDR_MODE ||
                addressMode == IMMEDIATE_ADDR_MODE) {
                
                doubleWord = readDoubleWord();
            }

            if (instruction == instructions["LOAD"]) {
                if (addressMode == REG_DIR_ADDR_MODE) {
                    coutString += "R" + to_string(R1Index);
                    REGISTER[R0Index] = REGISTER[R1Index];
                }
                else if (addressMode == REG_IND_ADDR_MODE) {
                    address = REGISTER[R1Index];
                    REGISTER[R0Index] = read(address, type);

                    coutString += "[R" + to_string(R1Index) + "]";
                }
                else if (addressMode == REG_IND_DISP_ADDR_MODE) {
                    if (R0Index != PC) {
                        address = REGISTER[R1Index] + doubleWord;
                    } else {
                        address = REGISTER[PC] + doubleWord - 8;
                    }
                    
                    REGISTER[R0Index] = read(address, type);
                    
                    coutString += "[R" + to_string(R1Index) + " + 0x" + to_string(doubleWord) + "]";
                }
                else if (addressMode == MEM_DIR_ADDR_MODE) {
                    address = doubleWord;
                    REGISTER[R0Index] = read(address, type);

                    coutString += "0x" + to_string(doubleWord);
                }
                else if (addressMode == IMMEDIATE_ADDR_MODE) {
                    REGISTER[R0Index] = doubleWord;

                    coutString += "#" + to_string(doubleWord);
                }
            }

            if (instruction == instructions["STORE"]) {
                if (addressMode == REG_DIR_ADDR_MODE) {
                    coutString += "R" + R1Index;
                    REGISTER[R1Index] = REGISTER[R0Index];
                }
                else if (addressMode == REG_IND_ADDR_MODE) {
                    address = REGISTER[R1Index];
                    write(REGISTER[R0Index], address, type);

                    coutString += "[R" + to_string(R1Index) + "]";
                }
                else if (addressMode == REG_IND_DISP_ADDR_MODE) {
                    if (R0Index != PC) {
                        address = REGISTER[R1Index] + doubleWord;
                    } else {
                        address = REGISTER[PC] + doubleWord - 8;
                    }

                    write(REGISTER[R0Index], address, type);

                    coutString += "[R" + to_string(R1Index) + " + 0x" + to_string(doubleWord) + "]";                
                }
                else if (addressMode == MEM_DIR_ADDR_MODE) {
                    address = doubleWord;
                    write(REGISTER[R0Index], address, type);

                    coutString += "0x" + to_string(doubleWord);                
                }
            }

            cout << coutString << endl;
        }
        
        //  Stack instructions
        if ((instruction >= instructions["PUSH"]) && (instruction <= instructions["POP"])) {

            cout << instructionString << " R" << R0Index << endl; 

            if (instruction == instructions["PUSH"]) {           
                push(R0Index);
            }
            else if (instruction == instructions["POP"]) {
                REGISTER[R0Index] = pop();
            }
        }

        //  Aritmetic and logic instructions
        if ((instruction >= instructions["ADD"]) && (instruction <= instructions["ASR"])) {
            cout << instructionString << " R" << R0Index << ", R" << R1Index << ", R" << R2Index << endl; 

            if (instruction == instructions["ADD"]) {
                REGISTER[R0Index] = REGISTER[R1Index] + REGISTER[R2Index];
            }
            else if (instruction == instructions["SUB"]) {
                REGISTER[R0Index] = REGISTER[R1Index] - REGISTER[R2Index];
            }
            else if (instruction == instructions["MUL"]) { 
                REGISTER[R0Index] = REGISTER[R1Index] * REGISTER[R2Index];
            }
            else if (instruction == instructions["DIV"]) {
                if (REGISTER[R2Index] == 0) {
                    errorDescription = "ERROR: DIV by ZERO";
                    return false;
                }

                REGISTER[R0Index] = REGISTER[R1Index] / REGISTER[R2Index];
            }
            else if (instruction == instructions["MOD"]) {
                if (REGISTER[R2Index] == 0) {
                    errorDescription = "ERROR: MOD by ZERO";
                    return false;
                }

                REGISTER[R0Index] = REGISTER[R1Index] % REGISTER[R2Index];
            }
            else if (instruction == instructions["AND"]) {
                REGISTER[R0Index] = REGISTER[R1Index] & REGISTER[R2Index];
            }
            else if (instruction == instructions["OR"]) {
                REGISTER[R0Index] = REGISTER[R1Index] | REGISTER[R2Index];
                }
            else if (instruction == instructions["XOR"]) {
                REGISTER[R0Index] = REGISTER[R1Index] ^ REGISTER[R2Index];
            }
            else if (instruction == instructions["NOT"]) {
                REGISTER[R0Index] = !REGISTER[R1Index];
            }
            else if (instruction == instructions["ASL"]) {
                REGISTER[R0Index] = REGISTER[R1Index] << REGISTER[R2Index];
            }
            else if (instruction == instructions["ASR"]) {
                REGISTER[R0Index] = REGISTER[R1Index] >> REGISTER[R2Index];
            }
        }
    }

    for (int i = 0x20; i < 0x44; i++) {
        cout << "MEMORY 0x" << std::hex << i <<  "=" << std::hex << (unsigned) MEMORY[i] << endl;
    }

    for (int i = 0x100; i < 0x100 + 16; i++) {
        cout << "MEMORY 0x" << std::hex << i <<  "=" << std::hex << (unsigned) MEMORY[i] << endl;
    }

    Emulator::finishExecution = true;

    return true;
}


