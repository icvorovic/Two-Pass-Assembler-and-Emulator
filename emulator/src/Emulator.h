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
#define PC 0x11     //  PC register index
#define SP 0x10     //  SP register index

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

    //  Update section symbol values
    void updateSectionSymbolValues(Section *section);

    //  Check sections intersection
    bool isSectionsIntersect();

    //  Get error description
    string getErrorDescription();

    //  Execute emulator memory content
    bool execute();

    //  Read by type
    long read(unsigned int address, unsigned int type);

    //  Read double word from PC address
    long readDoubleWord();

    //  Read double word from address
    long readDoubleWord(unsigned int address);

    //  Read unsigned word from address;
    long readUnsignedWord(unsigned int address);

    //  Read signed word from address
    long readSignedWord(unsigned int address);

    //  Read unsigned byte from address
    long readUnsignedByte(unsigned int address);

    //  Read signed byte from address
    long readSignedByte(unsigned int address);

    //  Write by type
    void write(int doubleWord, unsigned int address, unsigned int type);

    //  Write double word to address
    void writeDoubleWord(int doubleWord, unsigned int address);

    //  Write word to address
    void writeWord(int doubleWord, unsigned int address);

    //  Write byte to address
    void writeByte(int doubleWord, unsigned int address);

    //  Correct relocation memory location
    void correctRelocationMemory();

    //  Push register with index
    void push(unsigned int R0Index);
    
    //  Pop register with index
    long pop();
private:
    long REGISTER[18] = {};

    long doubleWord;

    unsigned char MEMORY[MAX_MEMORY_SIZE];

    Reader *reader;
    SymbolTable *symbolTable;
    vector<Section*> sectionArray;

    Section* currentSection;

    string errorDescription;
    bool error = false;
    static bool finishExecution;
};

#endif