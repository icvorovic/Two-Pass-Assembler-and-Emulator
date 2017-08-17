#include "RelocationTable.h"
#include <iostream>
#include <fstream>

RelocationTable::RelocationTable() {

}

RelocationTable::~RelocationTable() {
	relocationTableList.clear();
}

void RelocationTable::addRelocationRecord(RelocationTableEntry relocationRecord) {
	relocationTableList.push_back(relocationRecord);
}

void RelocationTable::writeToFile(string name) {
	ofstream file;
	
	file.open(name, std::ios::app);

	for (list<RelocationTableEntry>::iterator it = relocationTableList.begin(); it != relocationTableList.end(); ++it) {
		file << it->writeFormat() << endl;
	}

	file.close();
}