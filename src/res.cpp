#include <fstream>
#include <iostream>

#include "res.hpp"

const char* kerPath = "E:\\Benutzerdateien\\Documents\\GitHub\\black-hole-simulation\\src\\ker\\render.cl";
const char* kerOptsPath = "E:\\Benutzerdateien\\Documents\\GitHub\\black-hole-simulation\\cl_compile_options.txt";

const uint nKer = 3;
const char *kerNames[] = { "renderBg", "computePtProj", "drawPtShapes" };

const uint partRad0 = 3;