#include <fstream>
#include <iostream>

#include "res.hpp"

// OpenGL
const char* glPtShaderIns[] = {
    "pos",
    "color"
};
const char* glPtShaderUniforms[] = {
    "viewProj"
};

const char* glBgShaderIns[] = {
  "pos",
  "texCoord"
};

// OpenCL
const char* kernelNames[] = {
  "getPtPositions"
};