#include "Utility.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <map>
#include <stack>
#include <iostream>
#include <climits>
#include <regex>

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
	
vector<string> infixToPostfixExpression(string infix) {
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

	vector<string> postfix;
	string str;

	stringstream ss;

	cout << "LEN: " << infix.length() << endl;

	string number = "";

	for (int i = 0; i < infix.length(); i++) {
		char nextChar = infix.at(i);
		string next;

		if (nextChar == ' ') {
			continue;
		}

		ss = stringstream();

		ss << nextChar;
		ss >> next;

		std::cout << "NEXT : " << next << endl;

		if ((next.compare("+")) &&
			(next.compare("-")) &&
			(next.compare("*")) &&
			(next.compare("/")) &&
			(next.compare("(")) &&
			(next.compare(")"))) {
			postfix.push_back(next);
			rank++;
		}
		else {
			while (!stackPost.empty() && (inputPriority[next] <= stackPriority[stackPost.top()])) {
				str = stackPost.top();
				stackPost.pop();

				postfix.push_back(str);
				rank = rank + rang[str];

				if (rank < 1) {
					return vector<string>();
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
	}

	while (!stackPost.empty()) {
		str = stackPost.top();
		stackPost.pop();
		postfix.push_back(str);
		rank = rank + rang[str];
	}

	if (rank != 1) {
		return vector<string>();
	}

	return postfix;
}

int evaluateExpression(vector<string> postfix) {
	stack<int> stackExpression;
	stringstream ss;

	int result = 0;

	for (vector<string>::iterator it = postfix.begin(); it != postfix.end(); ++it) {
		string next = *it;

		if ((next.compare("+")) &&
			(next.compare("-")) &&
			(next.compare("*")) &&
			(next.compare("/"))) {
			
			regex binaryNumber("b[0-1]{1,32}");
			regex hexdecimalNumber("0x([0-9]{1,8})");
			regex decimalNumber("[1-9][0-9]*");

			int number = 0;
			string::size_type sz;
			
			if (regex_match(next, binaryNumber)) {
				number = stoi(next.substr(1, next.length()), nullptr, 2);
			}

			if (regex_match(next, hexdecimalNumber)) {
				number = stoi(next, nullptr, 0);
			}

			if (regex_match(next, decimalNumber)) {
				number = stoi(next, &sz);
			}

			stackExpression.push(number);
		}
		else {
			int firstOperand = stackExpression.top();
			stackExpression.pop();

			int secondOperand = stackExpression.top();
			stackExpression.pop();

			if (!next.compare("+")) {
				result = firstOperand + secondOperand;
			}

			if (!next.compare("-")) {
				result = firstOperand - secondOperand;
			}

			if (!next.compare("*")) {
				result = firstOperand * secondOperand;
			}

			if (!next.compare("/")) {
				result = firstOperand / secondOperand;
			}

			stackExpression.push(result);
		}
	}
	
	result = stackExpression.top();
	stackExpression.pop();

	if (stackExpression.empty()) {
		return result;
	} else {
		cout << "ERROR: Irregular expression" << endl;
		return INT_MAX;
	}
}