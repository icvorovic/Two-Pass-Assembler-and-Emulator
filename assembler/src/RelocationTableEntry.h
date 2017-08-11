#ifndef _RELOCATION_TABLE_ENTRY_H_
#define _RELOCATION_TABLE_ENTRY_H_

#include <string>

using namespace std;

class RelocationTableEntry {
public:
	//	Constructor
	RelocationTableEntry();

	//	Destructor
	~RelocationTableEntry();

	//	Set relocation table entry address
	void setAddress(unsigned int address);

	//	Get relocation table entry address
	unsigned int getAddress();

	//	Set relocation table entry type
	void setType(char type);

	//	Get relocation table entry type
	char getType();

	//	Set relocation table entry order number
	void setOrderNumber(unsigned int orderNumber);

	//	Get relocation table entry order number
	unsigned int getOrderNumber();

	//	Get string in format for write in file
	string writeFormat();
private:
	static unsigned int orderNumberGenerator;
	unsigned int address;
	char type;
	unsigned int orderNumber;
};

#endif