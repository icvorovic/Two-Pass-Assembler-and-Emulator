#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <map>
#include <vector>
#include <string>
#include <regex>

using namespace std;

/*
 *	Operation codes for every operation group
 */

const int IS_CONTROL_FLOW_OP = 0x00;
const int IS_LOAD_STORE_OP = 0x10;
const int IS_STACK_OP = 0x20;
const int IS_ARITM_LOG_OP = 0x30;

/*
 *	Regular expressions 
 */
const regex REGEX_LABEL("[a-z|A-Z|_][a-zA-Z0-9_]+:");
const regex REGEX_ADDR_MODE_IMMEDIATE("\\#([a-z|A-Z|_][a-zA-Z0-9_]+){0,1}([ ]*){0,1}([\\+\\-]){0,1}(.*){0,1}");
const regex REGEX_ADDR_MODE_REG_DIR("(R1[0-5]{1}|R[0-9]{1}|PC|SP){1}");
const regex REGEX_ADDR_MODE_MEM_DIR("[a-z|A-Z|_][a-zA-Z0-9_]+");
const regex REGEX_ADDR_MODE_REG_IND("\\[(R1[0-5]{1}|R[0-9]{1}|PC|SP){1}\\]");
const regex REGEX_ADDR_MODE_REG_IND_DISP("\\[(R1[0-5]{1}|R[0-9]{1}|PC|SP){1}(\\s)*\\+(\\s)*(.*)\\]");
const regex REGEX_ADDR_MODE_DOLLAR_PC("\\$([a-z|A-Z|_][a-zA-Z0-9_]+|0x[0-9]{0,8})");
const regex REGEX_CONST_EXPRESSION("([a-z|A-Z|_][a-zA-Z0-9_]+){0,1}([ ]*){0,1}([\\+\\-]){0,1}(.*){0,1}");

/*
*	Array contains all section types used in assembly language.
*/
extern vector<regex> sections;
extern map<string, int> instructions;

/*
*	Array contains all mnenomincs used in assembly language.
*/
 extern vector<string> mnemonics;
/*
 *	Array contains all directives for data defining used in assembly language.
 */
extern vector<string> dataDefining;
/*
 *	Array contains all directives used in assembly language.
 */
extern vector<string> directives;

/*
 *	Arrays contain mnemonics grouped by instruction types.
 */
extern vector<string> controlFlowInstructions;
extern vector<string> loadStoreInstructions;
extern vector<string> stackInstructions;
extern vector<string> aritmeticLogicInstructions;

/*
 *	Constants for different codes
 */

const int REG_DIR_ADDR_MODE = 0x0;
const int REG_IND_ADDR_MODE = 0x2;
const int REG_IND_DISP_ADDR_MODE = 0x7;
const int MEM_DIR_ADDR_MODE = 0x6;
const int IMMEDIATE_ADDR_MODE = 0x4;

const int DOUBLE_WORD = 0x0;
const int ZERO_WORD = 0x1;
const int SIGN_WORD = 0x5;
const int ZERO_BYTE = 0x3;
const int SIGN_BYTE = 0x7;

const int CONTROL_FLOW_INST_SIZE = 8;
const int LOAD_STORE_INST_SIZE = 8;
const int STACK_INST_SIZE = 4;
const int ARITM_LOGIC_INST_SIZE = 4;

const int OPCODE_OFFSET = 24;
const int ADDR_MODE_OFFSET = 21;
const int REG0_OFFSET = 16;
const int REG1_OFFSET = 11;
const int REG2_OFFSET = 6;
const int TYPE_OFFSET = 3;

extern map<string, int> registerCodes;

#endif
