#ifndef FILEMAN_HPP
#define FILEMAN_HPP

#include <string>

bool loadFile(const char* path, std::string* text, std::string* errMsg = nullptr);

#endif
