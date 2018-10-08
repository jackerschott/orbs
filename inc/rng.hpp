#ifndef RNG_HPP
#define RNG_HPP

#include <utility>
#include <climits>

typedef unsigned char byte;
typedef unsigned int uint;

uint rng();
uint randInt(uint max);
int randInt(int min, int max);
double randDouble();
double randDouble(double max);
double randDouble(double min, double max);
double normPdf(double d);
template<typename T> T selectObject(uint nObjects, std::pair<T, double> *collection);

#endif