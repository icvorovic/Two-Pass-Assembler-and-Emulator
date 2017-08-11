#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <list>
#include "SymbolTableEntry.h"

using namespace std;

class SymbolTable {
public:
	//	Constructor
	SymbolTable();

	//	Destructor
	~SymbolTable();

	//	Add new symbol in table
	void addSymbol(SymbolTableEntry &symbol);

	//	Write Symbol Table in file
	void writeToFile(string fileName);

	//	Find symbol by name
	SymbolTableEntry* findSymbolByName(string symbolName);
private:
	list<SymbolTableEntry> symbolTableList;
};
#endif