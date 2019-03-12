#include <vector>

#include "glprec.hpp"

namespace glprec {
  std::vector<const char*> names;
  std::vector<const char*> srcs;

  void addIncludeSrc(const char* name, const char* src) {
    names.push_back(name);
    srcs.push_back(src);
  }

  const char* preCompileShader(const char* shaderSrc) {
    
  }
}