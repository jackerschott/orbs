#ifndef RNG_HPP
#define RNG_HPP

#include <utility>
#include <climits>

typedef unsigned char byte;
typedef unsigned int uint;

uint rng();
uint randInt(uint max);
int randInt(int min, int max);
float randFloat();
float randFloat(float max);
float randFloat(float min, float max);
float normPdf(float d);
template<typename T> T selectObject(uint nObjects, std::pair<T, float> *collection) {
  //std::default_random_engine generator;
  //generator.seed(rng());
  //std::uniform_int_distribution<uint> distribution(0, UINT_MAX - 1);
  //uint selNumber = distribution(generator);
  uint selNumber = rng();
  uint probLimit = 0;
  for (uint i = 0; i < nObjects; i++) {
    probLimit += (uint)(UINT_MAX * collection[i].second);
    if (selNumber < probLimit) {
      return collection[i].first;
    }
  }
  return collection[nObjects - 1].first;
}

#endif