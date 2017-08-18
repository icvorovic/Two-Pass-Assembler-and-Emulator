#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

#include <vector>
#include <string>
#include "RelocationTable.h"
#include "SymbolTable.h"
#include "Section.h"
#include "Symbol.h"
#include "Reader.h"

using namespace std;

class Assembler {
public:
	//	Constructor
	Assembler(string inputFileName, string outputFileName);

	//	Destructor
	~Assembler();

	//	Assembler first pass
	bool firstPass();

	//	Assembler second pass
	bool secondPass();

	//	Reset all sections counters
	void resetSectionCounters();

	//	Check if section exists in section content array
	bool sectionExists(Section section);

	//	Find section by order number
	Section* findSectionByOrdNumber(unsigned int orderNumber);
	
	//	Find section by namespace
	Section* findSectionByName(string name);

	//	Create machine code for register indirect address mode
	unsigned long createCodeRegisterDirect(vector<string> arguments, int codeInstruction , int type);

	//	Create machine code for register indirect address mode
	unsigned long createCodeRegisterIndirect(vector<string>arguments, int codeInstruction, int type);
	
	//	Create machine code for register indirect with displacement address mode and create relocation record
	unsigned long long createCodeRegisterIndirectDisplacement(string argument, int codeInstruction, int type);
	
	//	Create machine code for pc relative with displacement address mode
	unsigned long long createCodePCRelative(string argument, int codeInstruction, int type);
	
	//	Write double word in string byte representation
	void writeSectionContent(string content);

private:
	string inputFileName;
	string outputFileName;

	vector<Section*> sectionArray;

	Reader *reader;
	Section *currentSection;

	SymbolTable symbolTable;
};

#endif