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
double randDouble() {
  return (double)rng() / UINT_MAX;
}
double randDouble(double max) {
  return (double)rng() / UINT_MAX * max;
}
double randDouble(double min, double max) {
  return (double)rng() / UINT_MAX * (max - min) + min;
}
double normPdf(double d) {
  std::default_random_engine generator;
  generator.seed(rng());
  std::normal_distribution<double> distribution(0, d);
  return distribution(generator);
}
template<typename T> T selectObject(uint nObjects, std::pair<T, double> *collection) {
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