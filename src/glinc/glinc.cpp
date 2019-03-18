#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string.h>

#include "glinc.hpp"

namespace glinc {
  std::map<std::string, std::string> incSrcs;

  unsigned long long nextInstr(std::string line, unsigned long long instrIndex);
  unsigned long long prevInstr(std::string line, unsigned long long instrIndex);
  unsigned long long instrLen(std::string line, unsigned long long instrIndex);

  void addIncludeSrc(const char* name, const char* src) {
    incSrcs.insert(std::make_pair(std::string(name), std::string(src)));
  }

  char* insertIncludes(const char* shaderSrc_) {
    std::string shaderSrc = std::string(shaderSrc_);

    std::string out;
    std::string line;
    std::istringstream ss(shaderSrc);
    while (std::getline(ss, line)) {
      unsigned long long statBeg = line.find('#');
      if (statBeg == std::string::npos) {
        out += line + '\n';
        continue;
      }

      unsigned long long incInstrBeg = line.length() > statBeg + 1 && !isspace(line[statBeg + 1]) ? statBeg + 1 : nextInstr(line, statBeg);
      if (statBeg == std::string::npos) {
        out += line + '\n';
        continue;
      }
      std::string incInstr = line.substr(incInstrBeg, instrLen(line, incInstrBeg));
      
      unsigned long long nameInstrBeg = nextInstr(line, incInstrBeg);
      if (nameInstrBeg == std::string::npos) {
        out += line + '\n';
        continue;
      }
      std::string nameInstr = line.substr(nameInstrBeg, instrLen(line, nameInstrBeg));
      std::string name = line.substr(nameInstrBeg + 1, nameInstr.length() - 2);

      if (incInstr == "include" && nameInstr[0] == '"' && nameInstr[nameInstr.length() - 1] == '"' && incSrcs.find(name) != incSrcs.end()) {
        std::string s1;
        std::string s2;
        unsigned long long nextInstrBeg;
        unsigned long long prevInstrEnd;
        if ((prevInstrEnd = prevInstr(line, statBeg)) != std::string::npos) {
          s1 = line.substr(0, prevInstrEnd + 1) + '\n';
        }
        if ((nextInstrBeg = nextInstr(line, nameInstrBeg)) != std::string::npos) {
          s2 = '\n' + line.substr(nextInstrBeg, line.length() - nextInstrBeg);
        }
        out += s1;

        std::istringstream incss(incSrcs.at(name));
        std::string incline;
        while (std::getline(incss, incline)) {
          out += incline + '\n';
        }
        out = out.substr(0, out.length() - 1);
        out += s2 + '\n';
      }
      else out += line + '\n';
    }
    out = out.substr(0, out.length() - 1);

    char* out_ = new char[out.length() + 1];
    strcpy(out_, out.c_str());
    return out_;
  }

  unsigned long long nextInstr(std::string line, unsigned long long instrIndex) {
    bool atOldInstr = true;
    for (unsigned long long i = instrIndex; i < line.length(); ++i) {
      if (atOldInstr && isspace(line[i])) {
        atOldInstr = false;
      }
      else if (!atOldInstr && !isspace(line[i])) {
        return i;
      }
    }
    return std::string::npos;
  }
  unsigned long long prevInstr(std::string line, unsigned long long instrIndex) {
    bool atOldInstr = true;
    for (unsigned long long i = instrIndex - 1; i < line.length(); --i) {
      if (atOldInstr && isspace(line[i])) {
        atOldInstr = false;
      }
      else if (!atOldInstr && !isspace(line[i])) {
        return i;
      }
    }
    return -1;
  }
  unsigned long long instrLen(std::string line, unsigned long long instrIndex) {
    for (unsigned long long i = instrIndex + 1; i < line.length(); ++i) {
      if (isspace(line[i])) {
        return i - instrIndex;
      }
    }
    return line.length() - instrIndex;
  }
}