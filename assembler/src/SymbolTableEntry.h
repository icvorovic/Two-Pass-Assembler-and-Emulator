#ifndef _SYMBOL_TABLE_ENTRY_H_
#define _SYMBOL_TABLE_ENTRY_H_

#include <string>

using namespace std;

class SymbolTableEntry{
public:
	//	Constructor
	SymbolTableEntry();

	//	Destructor
	virtual ~SymbolTableEntry();

	//	Set symbol type
	void setType(string type);

	//	Get symbol type
	string getType();

	//	Set order number
	void setOrderNumber(unsigned int num);

	//	Get order number
	unsigned int getOrderNumber();

	//	Set symbol name
	void setName(string name);

	//	Get symbol name
	string getName();

	//	Set section number
	void setSectionNumber(unsigned int num);

	//	Get section number
	unsigned int getSectionNumber();

	//	Set relative section offset
	void setSectionOffset(unsigned int offset);
	
	//	Get relative section offset
	unsigned int getSectionOffset();

	//	Get string in format for write in file
	virtual string formatWrite();
protected:
	string type;
	unsigned int orderNumber;
	string name;
	unsigned int sectionNumber;
	unsigned int offset;
};

#endif
