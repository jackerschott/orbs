#include <fstream>
#include <iostream>

#include "fileman.hpp"
#include "res.hpp"


bool loadFile(const char* path, std::string* text, std::string* errMsg) {
	std::string fileText;

	std::ifstream fileStream(path);
	if (fileStream.is_open()) {
    fileText = std::string(std::istreambuf_iterator<char>(fileStream), (std::istreambuf_iterator<char>()));
		fileStream.close();
	}
	else {
		if (errMsg != nullptr)
			*errMsg = std::string("Error while opening ") + path + "\n";
		*text = std::string();
		return false;
	}

	*text = fileText;
	return true;
}