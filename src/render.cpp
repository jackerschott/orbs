#include "../include/render.h"

double rs;
double rg;
uint nParticles;
particle* particles;
std::pair<color, double> *particleColorPalette;

double getRandom(double d);
template<typename T> T selectObject(uint nObjects, std::pair<T, double> *collection);

void initRender(double _rs, double _rg) {
  rs = _rs;
  rg = _rg;
}

void createParticleRing(uint rnParticles, double rr, double rtheta, double rphi,
  double rdr, double rdtheta, double rdphi,
  uint nColors, std::pair<color, double> *rparticleColorPalette) {

  particle* newParticles = new particle[nParticles + rnParticles];
  for (uint i = 0; i < nParticles; i++) {
    newParticles[i] = particles[i];
  }
  for (uint i = nParticles; i < nParticles + rnParticles; i++) {
    newParticles[i].r = rr + getRandom(rdr / 2.0);
    newParticles[i].phi = rphi + getRandom(rdphi / 2.0);
    newParticles[i].theta = rtheta + getRandom(rdtheta / 2.0);
    newParticles[i].vr = 0.0;
    newParticles[i].vphi = 1.0;
    newParticles[i].vtheta = 0.0;
    newParticles[i].pcolor = selectObject(nColors, rparticleColorPalette);
  }
  delete[] particles;
  particles = newParticles;
  nParticles += rnParticles;

  //for (int i = 0; i < nParticles; i++) {
  //  std::cout << particles[i].color.r << std::endl;
  //}
}

double getRandom(double d) {
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0, d);
  return distribution(generator);
}

template<typename T> T selectObject(uint nObjects, std::pair<T, double> *collection) {
  uint rn = rand();
  uint probLimit = 0;
  for (uint i = 0; i < nObjects; i++) {
    probLimit += ((uint)RAND_MAX + 1) / collection[i].second;
    if (rn < probLimit) {
      return collection[i].first;
    }
  }
  throw collection;
}
