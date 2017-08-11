#ifndef _SECTION_H_
#define _SECTION_H_

#include <string>
#include "SymbolTableEntry.h"

using namespace std;

class Section : public SymbolTableEntry {
public:
	//	Constructor
	Section();

	//	Copy constructor
	Section(const Section &sec);

	//	Destructor
	virtual ~Section();

	//	Set start address
	void setStartAddress(unsigned int address);

	//	Get start address
	unsigned int getStartAddress();

	//	Set section size
	void setSectionSize(unsigned int size);

	//	Get section size
	unsigned int getSectionSize();

	//	Set flags
	void setFlags(string flags);

	//	Get flags
	string getFlags();

	//	Get string in format for write in file
	string formatWrite();

	//	Increment location counter by number
	void incrementLocationCounterBy(unsigned int number);

	//	Reset location counter
	void resetLocationCounter();

	//	Get location counter
	unsigned int getLocationCounter();
private:
	unsigned int locationCounter;
	unsigned int startAddress;
	unsigned int sectionSize;
	string flags;
};

#endif
