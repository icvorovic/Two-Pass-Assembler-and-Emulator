#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

#include <vector>
#include <string>
#include "RelocationTable.h"
#include "SymbolTable.h"
#include "SectionContent.h"
#include "Section.h"
#include "Symbol.h"

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
private:
	string inputFileName;
	string outputFileName;

	vector<RelocationTable> relocationTableArray;
	vector<SectionContent> sectionContentArray;

	SymbolTable symbolTable;
};

#endif