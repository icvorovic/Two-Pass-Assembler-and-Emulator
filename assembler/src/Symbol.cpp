#include "Symbol.h"
#include "Utility.h"

Symbol::Symbol() {
	value = 0;
}

Symbol::~Symbol() {

}

//	Set symbol value
void Symbol::setValue(unsigned int val) {
	this->value = val;
}

//	Get symbol value
unsigned int Symbol::getValue() {
	return value;
}

//	Set section number
void Symbol::setFlag(char flag) {
	this->flag = flag;
}

//	Get section number
char Symbol::setFlag() {
	return flag;
}

string Symbol::formatWrite() {
	string str = this->SymbolTableEntry::formatWrite();
	str += "0x" + intToHexString(value) + " " + flag;
	return str;
}
