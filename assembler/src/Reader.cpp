#include <regex>
#include "Reader.h"

using namespace std;

Reader::Reader(string fileName) {
	this->fileName = fileName;
	fileStream.open(fileName);
}

Reader::~Reader() {
	fileStream.close();
}

string Reader::readNextLine() {
	if (!fileStream.is_open()) {
		cout << "File is not opened." << endl;
		return string();
	}
	
	if (fileStream.eof()) {
		cout << "End of file." << endl;
		
		fileStream.clear();
		fileStream.seekg(0, ios::beg);
	
		return string();
	}

	string str;

	getline(fileStream, str);

	if (!str.compare(".end")) {
		fileStream.clear();
		fileStream.seekg(0, ios::beg);
	}

	return str;
}

string Reader::trim(string &str) {
	str.erase(0, str.find_first_not_of(' '));
	str.erase(str.find_last_not_of(' ') + 1);
	str.erase(remove(str.begin(), str.end(), '\r'), str.end());
	return str;
}

vector<string> Reader::split(const string &str, char delimiter) {
	vector<std::string> tokens;
	size_t start = 0, end = 0;
	
	while ((end = str.find(delimiter, start)) != string::npos) {
		if (end != start) {
			tokens.push_back(str.substr(start, end - start));
		}
		start = end + 1;
	}

	if (end != start) {
		tokens.push_back(str.substr(start));
	}
	return tokens;
}

string Reader::discardComment(string &line) {
	if (line.find_last_of(';') != string::npos) {
		line.erase(line.find_last_of(';'), line.length());
	}
	return line;
}

string Reader::getFirstWord(string &line) {
	line = trim(line);

	string word;

	if (line.find(' ') == string::npos) {
		word = line.substr(0, line.find('\n'));
		line.erase(0, line.find('\n'));
	}
	else {
		word = line.substr(0, line.find(' '));
		line.erase(0, line.find(' '));
	}
	return word;
}

bool Reader::isInstruction(const string &str) {
	return find(mnemonics.begin(), mnemonics.end(), str) != mnemonics.end();
}

bool Reader::isLabel(const string &str) {
	return regex_match(str, REGEX_LABEL);
}

bool Reader::isSection(const string &str) {
	for (vector<regex>::iterator it = sections.begin(); it != sections.end(); ++it) {
		if (regex_match(str, *it)) {
			return true;
		}
	}
	return false;
}

bool Reader::isControlFlowInstruction(const string &str) {
	return find(controlFlowInstructions.begin(), controlFlowInstructions.end(), str) != controlFlowInstructions.end();
}

bool Reader::isLoadStoreInstruction(const string &str) {
	return find(loadStoreInstructions.begin(), loadStoreInstructions.end(), str) != loadStoreInstructions.end();
}

bool Reader::isStackInstruction(const string &str) {
	return find(stackInstructions.begin(), stackInstructions.end(), str) != stackInstructions.end();
}

bool Reader::isAritmeticLogicInstruction(const string &str) {
	return find(aritmeticLogicInstructions.begin(), aritmeticLogicInstructions.end(), str) != aritmeticLogicInstructions.end();
}
