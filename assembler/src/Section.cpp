#include "Section.h"
#include "Utility.h"
#include <iostream>

using namespace std;

Section::Section() {
	locationCounter = 0;
}

Section::Section(const Section &sec) {
	this->type = sec.type;
	this->orderNumber = sec.orderNumber;
	this->name = sec.name;
	this->sectionNumber = sec.sectionNumber;
	this->offset = sec.offset;
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

void Section::setContent(SectionContent content) {
	this->content = content;
}

SectionContent Section::getContent() {
	return content;
}

void Section::setRelocationTable(RelocationTable table) {
	this->relocationTable = table;
}

RelocationTable Section::getRelocationTable() {
	return relocationTable;
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

void Section::writeSectionContent(string contentStr) {
	for (int i = 0; i < 8; i++) {
		content.addByte(contentStr.at(i));
	}

	for (int i = 8; i < contentStr.size(); i++) {
		content.addByte(contentStr.at(i));
	}
}

void Section::addRelocationRecord(RelocationTableEntry entry) {
	relocationTable.addRelocationRecord(entry);
}
