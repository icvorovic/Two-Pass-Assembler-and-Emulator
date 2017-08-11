#include "Utility.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>

using namespace std;

string intToHexString(int num) {
	stringstream sstream;
	sstream << std::hex << num;
	std::string result = sstream.str();
	return result;
}

string byteToHexString(char byte) {
	return intToHexString((int)byte);
}

bool isFileExists(string fileName) {
	std::ifstream infile(fileName);
	return infile.good();
}