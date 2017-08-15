#include <string>
#include <iostream>
#include <regex>
#include "Assembler.h"
#include "Utility.h"

using namespace std;

int main(int argc, char** args) {
	//Assembler *as = new Assembler("test.txt", "blabla.txt");

	//as->firstPass();
	//as->secondPass();

	string infix;

	cin >> infix;

	vector<string> postfix =  infixToPostfixExpression(infix);

	cout << "rezultat" << evaluateExpression(postfix) << endl;
	//delete as;
}