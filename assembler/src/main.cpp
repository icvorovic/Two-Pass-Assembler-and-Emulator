#include <string>
#include <iostream>
#include <regex>
#include "Assembler.h"
#include "Utility.h"

using namespace std;

int main(int argc, char** args) {
	Assembler *as = new Assembler("test1.txt", "blabla.txt");

	bool result;
	
	result = as->firstPass();
	
	if (!result) {
		cout << as->getErrorDescription() << endl;
	} else {
		as->secondPass();
	}
	
	delete as;
}
