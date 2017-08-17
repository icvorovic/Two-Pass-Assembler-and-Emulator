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

void SectionContent::writeInFile(string name) {
	ofstream file;

	file.open(name, std::ios::app);

	int count = 0;
	const int BYTES_PER_LINE = 16;

	for (vector<char>::iterator it = byteContentArray.begin(); it != byteContentArray.end(); ++it) {
		file << byteToHexString(*it) << " ";

		count = (count + 1) % BYTES_PER_LINE;

		if (count == 0) {
			file << endl;
		}
	}
}

