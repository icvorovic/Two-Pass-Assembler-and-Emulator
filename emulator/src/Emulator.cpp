#include "Emulator.h"
#include "Constants.h"
#include "Symbol.h"
#include "Section.h"
#include <iostream>

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
                ||line.substr(0, relocationTablePrefix.size()) == sectionContentPrefix) {
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