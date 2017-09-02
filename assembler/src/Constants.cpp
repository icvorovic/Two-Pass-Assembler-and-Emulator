#include <map>
#include <vector>
#include <string>
#include <regex>
#include <iostream>
#include "Constants.h"

using namespace std;

vector<regex> sections({
	regex("\\.text(\\.(0|[1-9][0-9]*)){0,1}"),
	regex("\\.data(\\.(0|[1-9][0-9]*)){0,1}"),
	regex("\\.rodata(\\.(0|[1-9][0-9]*)){0,1}"),
	regex("\\.bss(\\.(0|[1-9][0-9]*)){0,1}")
});

static bool initializeInstuctionsMap(map<string, unsigned long> &instructions) {
	//	Control flow operations
	instructions["INT"] = 0x00;
	instructions["RET"] = 0x01;
	instructions["RTI"] = 0x01;
	instructions["JMP"] = 0x02;
	instructions["CALL"] = 0x03;
	instructions["JZ"] = 0x04;
	instructions["JNZ"] = 0x05;
	instructions["JGZ"] = 0x06;
	instructions["JGEZ"] = 0x07;
	instructions["JLZ"] = 0x08;
	instructions["JLEZ"] = 0x09;

	//	Load/Store operations
	instructions["LOAD"] = 0x10;
	instructions["LOADUB"] = 0x10;
	instructions["LOADSB"] = 0x10;
	instructions["LOADUW"] = 0x10;
	instructions["LOADSW"] = 0x10;
	
 	instructions["STORE"] = 0x11;
	instructions["STOREB"] = 0x11;
	instructions["STOREW"] = 0x11;
	
	//	Stack operations
	instructions["PUSH"] = 0x20;
	instructions["POP"] = 0x21;

	//	Aritmetic and Logic operations
	instructions["ADD"] = 0x30;
	instructions["SUB"] = 0x31;
	instructions["MUL"] = 0x32;
	instructions["DIV"] = 0x33;
	instructions["MOD"] = 0x34;
	instructions["AND"] = 0x35;
	instructions["OR"] = 0x36;
	instructions["XOR"] = 0x37;
	instructions["NOT"] = 0x38;
	instructions["ASL"] = 0x39;
	instructions["ASR"] = 0x3A;

	return true;
}

map<string, unsigned long> instructions;
bool initResult = initializeInstuctionsMap(instructions);

static bool initializeRegisterCodesMap(map<string, int> &registerCodes) {
	int i;

	for (i = 0; i < 16; i++) {
		registerCodes["R" + to_string(i)] = i;
	}

	registerCodes["SP"] = 0x10;
	registerCodes["PC"] = 0x11;

	return true;
}

map<string, int> registerCodes;
bool initResultReg = initializeRegisterCodesMap(registerCodes);

/*
*	Array contains all mnenomincs used in assembly language.
*/
vector<string> mnemonics({
	"INT",
	"RET",
	"RTI",
	"JMP",
	"CALL",
	"JZ",
	"JNZ",
	"JGZ",
	"JGEZ",
	"JLZ",
	"JLEZ",
	"LOAD",
	"LOADUB",
	"LOADSB",
	"LOADUW",
	"LOADSW",
	"STORE",
	"STOREB",
	"STOREW",
	"PUSH",
	"POP",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"MOD",
	"AND",
	"OR",
	"XOR",
	"NOT",
	"ASL",
	"ASR"
});

/*
*	Arrays contain mnemonics grouped by instruction types.
*/
vector<string> controlFlowInstructions({
	"INT",
	"RET",
	"RTI",
	"JMP",
	"CALL",
	"JZ",
	"JNZ",
	"JGZ",
	"JGEZ",
	"JLZ",
	"JLEZ"
});

vector<string> loadStoreInstructions({
	"LOAD",
	"LOADUB",
	"LOADSB",
	"LOADUW",
	"LOADSW",
	"STORE",
	"STOREB",
	"STOREW"
});

vector<string> stackInstructions({
	"PUSH",
	"POP"
});

vector<string> aritmeticLogicInstructions({
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"MOD",
	"AND",
	"OR",
	"XOR",
	"NOT",
	"ASL",
	"ASR"
});

/*
*	Array contains all directives for data defining used in assembly language.
*/
vector<string> dataDefining({
	"DB",
	"DW",
	"DD",
	"DUP"
});

/*
*	Array contains all directives used in assembly language.
*/
vector<string> directives({
	"DEF",
	"ORG",
	".global"
});
