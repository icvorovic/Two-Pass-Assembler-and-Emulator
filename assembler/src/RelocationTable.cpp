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

void RelocationTable::writeToFile(string sectionName, string name) {
	ofstream file;
	
	if (relocationTableList.size() == 0) {
		return;
	}

	file.open(name, std::ios::app);

	file << "#rel" + sectionName <<endl;

	for (list<RelocationTableEntry>::iterator it = relocationTableList.begin(); it != relocationTableList.end(); ++it) {
		file << it->writeFormat() << endl;
	}

	file.close();
}