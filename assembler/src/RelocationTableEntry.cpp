#include "RelocationTableEntry.h"
#include "Utility.h"

unsigned int RelocationTableEntry::orderNumberGenerator = 0;

RelocationTableEntry::RelocationTableEntry() {
	orderNumber = ++orderNumberGenerator;
}

RelocationTableEntry::~RelocationTableEntry() {

}

void RelocationTableEntry::setAddress(unsigned int address) {
	this->address = address;
}

unsigned int RelocationTableEntry::getAddress() {
	return address;
}

void RelocationTableEntry::setType(char type) {
	this->type = type;
}

char RelocationTableEntry::getType() {
	return type;
}

void RelocationTableEntry::setOrderNumber(unsigned int orderNumber) {
	this->orderNumber = orderNumber;
}

unsigned int RelocationTableEntry::getOrderNumber() {
	return orderNumber;
}

string RelocationTableEntry::writeFormat() {
	return "0x" + intToHexString(address) + " " + type + " " + to_string(orderNumber);
}