#include "render.h"
#include <random>

void initRender(double _rs, double _gr) {

}

void createParticleRing(int nParticles, double rr, double rtheta, double rphi,
	double rdr, double rdtheta, double rdphi,
	std::pair<color, double> *rparticleColorPalette) {
	particle particles[nParticles];
	for (int i = 0; i < nParticles; ++i)
	{
		particles[i].r = rr + getRandom(rdr);
	}
}

double getRandom(double d) {
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(0, d);
	return distribution(generator);
}
