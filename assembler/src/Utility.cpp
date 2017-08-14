#include "Utility.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <map>
#include <stack>
#include <iostream>

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
	
string infixToPostfixExpression(string infix) {
	map<string, int> inputPriority;

	inputPriority["+"] = inputPriority["-"] = 2;
	inputPriority["*"] = inputPriority["/"] = 3;
	inputPriority["("] = 6;
	inputPriority[")"] = 1; 

	map<string, int> stackPriority;

	stackPriority["+"] = stackPriority["-"] = 2;
	stackPriority["*"] = stackPriority["/"] = 3;
	stackPriority["("] = 0;

	map<string, int> rang;

	rang["+"] = rang["-"] = -1;
	rang["*"] = rang["/"] = -1;

	stack<string> stackPost;

	int rank = 0;

	string postfix = "";
	string str;

	stringstream ss;

	for (int i = 0; i < infix.length(); i++) {
		char nextChar = infix.at(i);
		string next;

		ss << nextChar;
		ss >> next;

		std::cout << next << endl;

		if ((next.compare("+")) &&
			(next.compare("-")) &&
			(next.compare("*")) &&
			(next.compare("/")) &&
			(next.compare("(")) &&
			(next.compare(")"))) {
			postfix += next;
			rank++;
		}
		else {
			while (!stackPost.empty() && inputPriority[next] <= stackPriority[stackPost.top()]) {
				str = stackPost.top();
				stackPost.pop();

				postfix += str;
				rank = rank + rang[str];

				if (rank < 1) {
					return string();
				}
			}

			if (next.compare(")")) {
				stackPost.push(next);
			}
			else {
				str = stackPost.top();
				stackPost.pop();
			}
		}

		std::cout << postfix << endl;
	}

	while (!stackPost.empty()) {
		str = stackPost.top();
		stackPost.pop();
		postfix += str;
		rank = rank + 1;
	}

	if (rank != 1) {
		return string();
	}

	return postfix;
}