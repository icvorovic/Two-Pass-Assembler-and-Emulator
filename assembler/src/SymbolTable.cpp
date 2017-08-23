#include "SymbolTable.h"
#include <iostream>
#include <fstream>

SymbolTable::SymbolTable() {

}

SymbolTable::~SymbolTable() {
	symbolTableList.clear();
}

void SymbolTable::addSymbol(SymbolTableEntry *symbolEntry) {
	symbolTableList.push_back(symbolEntry);
}

void SymbolTable::writeToFile(string name) {
	ofstream file;

	file.open(name, std::ios::app);

	file << "#TabelaSimbola" << endl;

	for (list<SymbolTableEntry*>::iterator it = symbolTableList.begin(); it != symbolTableList.end(); ++it) {
		file << (*it)->formatWrite() << endl;
	}

	file.close();

}

SymbolTableEntry* SymbolTable::findSymbolByName(string symbolName) {
	for (list<SymbolTableEntry*>::iterator it = symbolTableList.begin(); it != symbolTableList.end(); ++it) {
		string name = (*it)->getName();

		if (name.compare(symbolName) == 0) {
			return *it;
		}
	}

	return nullptr;
}

vector<Symbol*> SymbolTable::getSectionSymbols(Section *section) {
	unsigned int orderNumber = section->getOrderNumber();

	vector<Symbol*> result;

	for (list<SymbolTableEntry*>::iterator it = symbolTableList.begin(); it != symbolTableList.end(); ++it) {
		if ((*it)->getSectionNumber() == orderNumber && (*it) != section) {
			result.push_back((Symbol*)(*it));
		}
	}

	return result;
}