#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include <string>
#include <iostream>
#include "SymbolTableEntry.h"

using namespace std;

class Symbol : public SymbolTableEntry {
public:
	//	Constructor
	Symbol();

	//	Destructor
	virtual ~Symbol();

	//	Set symbol value
	void setValue(unsigned int name);

	//	Get symbol value
	unsigned int getValue();

	//	Set section number
	void setFlag(char num);

	//	Get section number
	char setFlag();

	//	Get string in format for write in file
	string formatWrite();

private:
	unsigned int value;
	char flag;
};

#endif
