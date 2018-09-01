#include "../hdr/render.h"

double rs;
double gr;
int nParticles;
particle* particles;
std::pair<color, double> *particleColorPalette;

void initRender(double _rs, double _gr) {

}

void createParticleRing(int rnParticles, double rr, double rtheta, double rphi,
  double rdr, double rdtheta, double rdphi,
  std::pair<color, double> *rparticleColorPalette) {

  particle* newParticles = new particle[nParticles + rnParticles];
  memcpy(newParticles, particles, nParticles);
  for (int i = nParticles; i < nParticles + rnParticles; ++i)
  {
    newParticles[i].r = rr + getRandom(rdr / 2.0);
    newParticles[i].phi = rphi + getRandom(rdphi / 2.0);
    newParticles[i].theta = rtheta + getRandom(rdtheta / 2.0);
  }
  delete[] particles;
  particles = newParticles;
  nParticles += rnParticles;


}

double getRandom(double d) {
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0, d);
  return distribution(generator);
}
