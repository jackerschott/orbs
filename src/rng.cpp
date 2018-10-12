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
float randFloat() {
  return (float)rng() / UINT_MAX;
}
float randFloat(float max) {
  return (float)rng() / UINT_MAX * max;
}
float randFloat(float min, float max) {
  return (float)rng() / UINT_MAX * (max - min) + min;
}
float normPdf(float d) {
  std::default_random_engine generator;
  generator.seed(rng());
  std::normal_distribution<float> distribution(0, d);
  return distribution(generator);
}