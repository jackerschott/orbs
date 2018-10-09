#ifndef RNG_HPP
#define RNG_HPP

#include <utility>
#include <climits>

typedef unsigned char byte;
typedef unsigned int uint;

uint rng();
uint randInt(uint max);
int randInt(int min, int max);
float randDouble();
float randDouble(float max);
float randDouble(float min, float max);
float normPdf(float d);
template<typename T> T selectObject(uint nObjects, std::pair<T, float> *collection);

#endif