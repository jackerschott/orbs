#include <random>
#include "../hdr/render.h"

void initRender(double _rs, double _gr) {
  //
}

void createParticleRing(int nParticles, double rr, double rtheta, double rphi,
	double rdr, double rdtheta, double rdphi,
	std::pair<color, double> *rparticleColorPalette) {
	particle* particles;
	for (int i = 0; i < nParticles; ++i)
	{
		particles[i].r = rr + getRandom(rdr / 2.0);
		particles[i].phi = rphi + getRandom(rdphi / 2.0); 
		particles[i].theta = rtheta + getRandom(rdtheta / 2.0);
	}

	ring r = { particles };
	ring* newRings = new ring[nRings + 1];
	memcpy(newRings, rings, nRings);
	newRings[nRings] = r;
	delete[] rings;
	rings = newRings;
	nRings++;
}

double getRandom(double d) {
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(0, d);
	return distribution(generator);
}
