#ifndef _RELOCATION_TABLE_H_
#define _RELOCATION_TABLE_H_

#include <list>
#include <string>
#include "RelocationTableEntry.h"

using namespace std;

class RelocationTable {
public:
	//	Constructor 
	RelocationTable();

	//	Destructor
	~RelocationTable();

	//	Add new relocation record
	void addRelocationRecord(RelocationTableEntry relocationRecord);

	//	Write relocation table in file
	void writeToFile(string sectionName, string name);

	//	Get relocation table list
	list<RelocationTableEntry> getRelocationList();

private:
	list<RelocationTableEntry> relocationTableList;
};
#endif