#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

#include <vector>
#include <string>
#include "RelocationTable.h"
#include "SymbolTable.h"
#include "SectionContent.h"
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

private:
	string inputFileName;
	string outputFileName;

	vector<RelocationTable> relocationTableArray;
	vector<SectionContent> sectionContentArray;

	Reader *reader;

	SymbolTable symbolTable;
};

#endif