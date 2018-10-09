#include <fstream>
#include <iostream>

#include "fileman.hpp"
#include "res.hpp"


bool loadFile(const char* path, std::string& text, std::string* errMsg) {
	std::string fileText;

	std::ifstream fileStream(path, std::ios::ate);
	if (fileStream.is_open()) {
		uint textSize = (uint)fileStream.tellg();
		char* text = new char[textSize];
		fileStream.seekg(0, std::ios::beg);
		fileStream.read(text, textSize);
		fileStream.close();

		fileText = std::string(text, textSize);
		delete[] text;
	}
	else {
		if (errMsg != nullptr)
			*errMsg = std::string("Error while opening ") + path + "\n";
		text = std::string();
		return false;
	}

	text = fileText;
	return true;
}