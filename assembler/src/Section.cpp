#include "Section.h"
#include "Utility.h"

Section::Section() {
	locationCounter = 0;
}

Section::Section(const Section &sec) {
	this->locationCounter = sec.locationCounter;
	this->startAddress = sec.startAddress;
	this->sectionSize = sec.sectionSize;
	this->flags = sec.flags;
}

Section::~Section() {

}

void Section::setStartAddress(unsigned int address) {
	this->startAddress = address;
}

unsigned int Section::getStartAddress() {
	return startAddress;
}

void Section::setSectionSize(unsigned int size) {
	this->sectionSize = size;
}

unsigned int Section::getSectionSize() {
	return sectionSize;
}

void Section::setFlags(string flags) {
	this->flags = flags;
}

string Section::getFlags() {
	return flags;
}

string Section::formatWrite() {
	string str = SymbolTableEntry::formatWrite();
	str += "0x" + intToHexString(startAddress) + " " + "0x" + intToHexString(sectionSize) + " " + flags;
	return str;
}

void Section::incrementLocationCounterBy(unsigned int number) {
	locationCounter = locationCounter + number;
}

void Section::resetLocationCounter() {
	locationCounter = 0;
}

unsigned int Section::getLocationCounter() {
	return locationCounter;
}
