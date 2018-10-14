#include <fstream>
#include <iostream>

#include "res.hpp"

// OpenGL
const char* glPtShaderInNames[] = {
    "pos",
    "color"
};
const char* glPtShaderUniNames[] = {
    "viewProj"
};

const char* glBgShaderInNames[] = {
  "pos",
  "texCoord"
};

// OpenCL
const char* kernelNames[] = {
  "getPtPositions",
  "processImage"
};

namespace std {
  const char* invalid_operation::what() {
    return "This operation is invalid in the current state.";
  }
}