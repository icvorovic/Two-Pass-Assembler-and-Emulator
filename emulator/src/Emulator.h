#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <string>
#include <vector>
#include "Reader.h"
#include "RelocationTable.h"
#include "Section.h"
#include "SymbolTable.h"

using namespace std;

#define MAX_MEMORY_SIZE 67108864   //  64MB

class Reader;
class RelocationTable;
class Section;
class SymbolTable;

class Emulator {
public:
    //  Constructor
    Emulator(string inputFile);

    //  Destructor
    ~Emulator();

    //  Get instruction code from machine code
    unsigned long getInstuctionCode();

    //  Get address mode from machine code
    unsigned long getAddressModeCode();

    //  Get REG0 code from machine code
    unsigned long getR0Code();

    //  Get REG1 code from machine code
    unsigned long getR1Code();

    //  Get REG2 code from machine code
    unsigned long getR2Code();

    //  Get type code from machine code
    unsigned long getTypeCode();

    //  Read symbol table, relocation table and section content from input file
    bool readInputStructures();

    //  Fill memory with section byte content
    bool fillMemory();
private:
    unsigned long REGISTER[16];
    unsigned long PC;
    unsigned long SP;

    unsigned long doubleWord;

    char MEMORY[MAX_MEMORY_SIZE];

    Reader *reader;
    SymbolTable *symbolTable;
    vector<Section*> sectionArray;

    Section* currentSection;
};

#endif