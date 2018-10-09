#include "randutils.hpp"

#include "rng.hpp"

uint rng()
{
  static uint counter = 0;
  randutils::seed_seq_fe256 seeder{ counter, 0u, 0u, 0u };
  counter++;
  uint out;
  seeder.generate(&out, &out + 1);
  return out;
}
uint randInt(uint max) {
  return rng() % max + 1;
}
int randInt(int min, int max) {
  return rng() % (max - min) + min + 1;
}
float randDouble() {
  return (float)rng() / UINT_MAX;
}
float randDouble(float max) {
  return (float)rng() / UINT_MAX * max;
}
float randDouble(float min, float max) {
  return (float)rng() / UINT_MAX * (max - min) + min;
}
float normPdf(float d) {
  std::default_random_engine generator;
  generator.seed(rng());
  std::normal_distribution<float> distribution(0, d);
  return distribution(generator);
}
template<typename T> T selectObject(uint nObjects, std::pair<T, float> *collection) {
  std::default_random_engine generator;
  generator.seed(rng());
  std::uniform_int_distribution<uint> distribution(0, UINT_MAX - 1);
  uint selNumber = distribution(generator);
  uint probLimit = 0;
  for (uint i = 0; i < nObjects; i++) {
    probLimit += UINT_MAX * collection[i].second;
    if (selNumber < probLimit) {
      return collection[i].first;
    }
  }
  throw "test";
}