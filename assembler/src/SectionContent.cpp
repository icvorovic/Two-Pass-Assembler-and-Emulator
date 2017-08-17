#include "SectionContent.h"
#include "Utility.h"

#include <iostream>
#include <fstream>

SectionContent::SectionContent() {

}

SectionContent::~SectionContent() {

}

void SectionContent::addByte(char byte) {
	byteContentArray.push_back(byte);
}

void SectionContent::writeInFile(string sectionName, string name) {
	ofstream file;

	file.open(name, std::ios::app);

	int count = 0;
	const int BYTES_PER_LINE = 32;

	file << "<" << sectionName << ">" << endl;

	for (int i = 0; i < byteContentArray.size(); i++) {
		file << byteContentArray[i];
		
		if ((i % 2 == 1) && (count != 0) && ((i + 1) % BYTES_PER_LINE != 0)) {
			file << " ";
		}

		count = (count + 1) % BYTES_PER_LINE;

		if (count == 0) {
			file << endl;
		}
	}

	file << endl;
}

