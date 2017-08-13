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

string longlongToHexString(unsigned long long number, int bytesNumber) {
		string result = "";
		
        for(int i = 0; i < 2 * bytesNumber; i++){
            unsigned long long tmp = number & 0xFFFF;
			
			tmp += tmp >= 10 ? 55 : 48;
			
			result = (char)tmp + result;
	
			number >>= 4;
		}
		
        return result;
    }