#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <vector>

#include "glinc.hpp"

namespace glprec {
  std::set<std::string> names;
  std::vector<std::string> srcs;

  char* toNextStat(char* stat);
  char* toBegOfStat(char* stat);
  char* toEndOfStat(char* stat);

  void addIncludeSrc(std::string name, std::string src) {
    names.insert(name);
    srcs.push_back(src);
  }

  std::string shaderIncludes(std::string shaderSrc) {
    const char* key = "include";
    char* line = strtok(shaderSrc, "\n");

    char* out = (char*)malloc(strlen(shaderSrc) + 1);
    out[0] = '\0';
    while (line != NULL) {
      char* directBeg = strchr(line, '#');
      line = strtok(NULL, "\n");

      char* keyBeg;
      char* nameBeg;
      char* nameEnd;
      if (directBeg != NULL
        && (keyBeg = toNextStat(directBeg)) != NULL
        && strcmp(keyBeg, key)
        && (nameBeg = toNextStat(keyBeg)) != NULL
        && nameBeg[0] == '"'
        && (nameEnd = toEndOfStat(nameBeg)) != NULL
        && nameEnd[0] == '"'
        && ) {
        
      }

      strcat(out, line);
      strcat(out, "\n");
    }
  }

  char* toNextStat(char* stat) {
    bool atOldStat;
    for (int i = 0; i < strlen(stat); ++i) {
      if (atOldStat && !isprint(stat[i])) {
        atOldStat = false;
      }
      if (!atOldStat && isprint(stat[i])) {
        return stat + i;
      }
    }
    return NULL;
  }
  char* toEndOfStat(char* stat) {
    for (int i = 0; i < strlen(stat); ++i) {
      if (!isprint(stat[i])) {
        return stat + i - 1;
      }
    }
    return NULL;
  }
}