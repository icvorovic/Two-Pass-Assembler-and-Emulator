#ifndef _SECTION_CONTENT_H_
#define _SECTION_CONTENT_H_

#include <vector>
#include <string>
#include "Section.h"

using namespace std;

class SectionContent {
public:
	//	Constructor
	SectionContent(Section section);

	//	Destructor
	~SectionContent();

	//	Add new byte to section content
	void addByte(char byte);

	//	Write byte vector to file in HEX format
	void writeInFile(string name);

	//	Set section
	void setSection(Section &section);

	//	Get section
	Section getSection();
private:
	Section section;
	vector<char> byteContentArray;
};
#endif