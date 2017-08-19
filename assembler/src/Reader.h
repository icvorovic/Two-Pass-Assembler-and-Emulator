#ifndef _READER_H_
#define _READER_H_

#include <string>
#include <fstream>
#include <iostream>
#include "Constants.h"

using namespace std;

class Reader {
public:
	//	Constructor
	Reader(string fileName);

	//	Destructor
	~Reader();

	//	Read next line from file
	string readNextLine();
	
	//	Trim whitespaces from string
	string trim(string &str);

	//	Split string by delimiter
	std::vector<std::string> split(const std::string &str, char delimiter);

	//	Discard comment if exists in line
	string discardComment(string &line);

	//	Remove and get first word from string line
	string getFirstWord(string &line);

	//	Check if string is instuction mnemonic
	bool isInstruction(const string &str);

	//	Check if string is label
	bool isLabel(const string &str);

	//	Check if string is section
	bool isSection(const string &str);

	//	Check if instruction is control flow instruction
	bool isControlFlowInstruction(const string &str);

	//	Check if instruction is load or store instruction
	bool isLoadStoreInstruction(const string &str);

	//	Check if instruction is stack instruction
	bool isStackInstruction(const string &str);

	//	Check if instruction is aritmetic-logic instruction
	bool isAritmeticLogicInstruction(const string &str);

	bool isOrgDirective(const string &str);
private:
	string fileName;
	ifstream fileStream;
};
#endif



