#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string.h>

#include "gl.hpp"

namespace gl {
  bool readShaderSrc(const char* path, std::string& src);
  bool writeShaderSrc(const char* path, std::string src);
  unsigned long long nextInstr(std::string line, unsigned long long instrIndex);
  unsigned long long prevInstr(std::string line, unsigned long long instrIndex);
  unsigned long long instrLen(std::string line, unsigned long long instrIndex);

  shader::shader(GLenum type) : type(type) {
    id = glCreateShader(type);
  }
  shader::shader(GLenum type, const char* srcPath) : type(type) {
    id = glCreateShader(type);
    setSrc(srcPath);
  }

  shader::~shader() {
    glDeleteShader(id);
  }

  void shader::setIncludeSrc(const char* name, const char* srcPath) {
    std::string incSrc;
    if (!readShaderSrc(srcPath, incSrc)) {
      std::cerr << srcPath << " could not be opened." << std::endl;
    }
    incSrcs.insert(std::make_pair(std::string(name), incSrc));
  }

  void shader::setSrc(const char* srcPath) {
    std::string srcPath_ = std::string(srcPath);
    fileName = srcPath_.substr(srcPath_.find_last_of("/") + 1);
    if (!readShaderSrc(srcPath, src)) {
      std::cerr << srcPath << " could not be opened." << std::endl;
      throw;
    }
  }

  std::string shader::insertIncludes(std::string shaderSrc) {
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
    return out;
  }
  
  bool shader::compile(std::string* log) {
    int success;
    char log_[0x400];

    std::string src_ = insertIncludes(src);
    writeShaderSrc(("obj/" + fileName).c_str(), src_);

    const char* src__ = src_.c_str();
    int len__ = src_.length();
    glShaderSource(id, 1, &src__, &len__);

    glCompileShader(id);
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(id, sizeof(log_), NULL, log_);
      if (log != NULL)
        *log = std::string(log_);
    }
    return success;
  }

  bool readShaderSrc(const char* path, std::string& src) {
    std::ifstream fs;
    fs.open(path);

    std::string line;
    if (fs.is_open()) {
      while (std::getline(fs, line)) {
        src += line + '\n';
      }
    }
    else return false;

    fs.close();
    return true;
  }
  bool writeShaderSrc(const char* path, std::string src) {
    std::ofstream fs;
    fs.open(path);
    if (!fs.is_open())
      return false;
    fs << src;
    fs.close();
    return true;
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