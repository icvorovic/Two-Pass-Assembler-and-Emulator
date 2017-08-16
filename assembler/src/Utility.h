#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <string>
#include <vector>

using namespace std;

string intToHexString(int num);

string byteToHexString(char byte);

bool isFileExists(string fileName);

string longlongToHexString(unsigned long long decimal, int b);

vector<string> infixToPostfixExpression(string infix);

int evaluateExpression(vector<string> postfix);

bool isOperand(string str);

#endif