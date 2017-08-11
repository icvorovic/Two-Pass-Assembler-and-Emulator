#include "SymbolTableEntry.h"

unsigned int SymbolTableEntry::orderNumberGenerator = 0;

SymbolTableEntry::SymbolTableEntry() {
	orderNumber = ++orderNumberGenerator;
}

SymbolTableEntry::~SymbolTableEntry() {
	
}

void SymbolTableEntry::setType(string type) {
	this->type = type;
}

string SymbolTableEntry::getType() {
	return type;
}

void SymbolTableEntry::setOrderNumber(unsigned int num) {
	this->orderNumber = num;
}

unsigned int SymbolTableEntry::getOrderNumber() {
	return orderNumber;
}

void SymbolTableEntry::setName(string name) {
	this->name = name;
}

string SymbolTableEntry::getName() {
	return name;
}

void SymbolTableEntry::setSectionNumber(unsigned int num) {
	this->sectionNumber = num;
}

unsigned int SymbolTableEntry::getSectionNumber() {
	return sectionNumber;
}

void SymbolTableEntry::setSectionOffset(unsigned int offset) {
	this->offset = offset;
}

unsigned int SymbolTableEntry::getSectionOffset() {
	return offset;
}

string SymbolTableEntry::formatWrite() {
	string str;
	str = type + " " + to_string(orderNumber) + " " + name + " " + to_string(sectionNumber) + " ";
	return str;
}