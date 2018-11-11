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
const char* ptgenKernelNames[] = {
  "getEllipticPtDistr",
  "getPtColors"
};
const char* rngKernelNames[] = {
  "genSamples",
  "genFloatSamples",
  "genGaussianSamples"
};
