#ifndef _SECTION_CONTENT_H_
#define _SECTION_CONTENT_H_

#include <vector>
#include <string>

using namespace std;

class SectionContent {
public:
	//	Constructor
	SectionContent();

	//	Destructor
	~SectionContent();

	//	Add new byte to section content
	void addByte(char byte);

	//	Write byte vector to file in HEX format
	void writeInFile(string sectionName, string name);

	//	Get section content like byte array
	vector<char> getByteContent();

private:
	vector<char> byteContentArray;
};
#endif