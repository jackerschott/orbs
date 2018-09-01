#ifndef RENDER_H
#define RENDER_H

#include <random>

typedef unsigned char byte;

struct color {
  byte r;
  byte g;
  byte b;
};

struct particle {
  double r;
  double theta;
  double phi;
  double vr;
  double vtheta;
  double vphi;
  color pcolor;
  particle* children;
};

void initRender(double _rs, double _gr);
void createParticleRing(unsigned int nParticles, double rr, double rphi, double rtheta,
  double rdr, double rdphi, double rdtheta,
  std::pair<color, double> *rparticleColorPalette);

#endif
