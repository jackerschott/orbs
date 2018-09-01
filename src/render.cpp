#include <random>

struct color {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct particle {
  double r;
  double theta;
  double phi;
  double vr;
  double vtheta;
  double vphi;
  color pcolor;
};

double ranval(double d) {
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0, d);
  return distribution(generator);
}

void createParticleRing(double rr, double rdr, double rtheta, double rdtheta, double rphi, double rdphi,
                        unsigned int nop, color color1, color color2, color color3, color color4, color color5) {
  particle particles[nop];
  for(unsigned int n = 0; n < nop; n = n + 1)
  {
    particles[n].r = rr + ranval(rdr);
  }
}
