#include "SectionContent.h"
#include "Utility.h"

#include <iostream>
#include <fstream>

SectionContent::SectionContent(Section section) {
	this->section = section;
}

SectionContent::~SectionContent() {

}

void SectionContent::setSection(Section &section) {
	this->section = section;
}

Section SectionContent::getSection() {
	return section;
}

void SectionContent::addByte(char byte) {
	byteContentArray.push_back(byte);
}

void SectionContent::writeInFile(string name) {
	ofstream file;

	file.open(name);

	int count = 0;
	const int BYTES_PER_LINE = 16;

	file << "<" << section.getName() << ">" << endl;

	for (vector<char>::iterator it = byteContentArray.begin(); it != byteContentArray.end(); ++it) {
		file << byteToHexString(*it) << " ";

		count = (count + 1) % BYTES_PER_LINE;

		if (count == 0) {
			file << endl;
		}
	}
}

