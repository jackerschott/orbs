#ifndef RENDER_H
#define RENDER_H

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
};

double rs;
double gr;
std::pair<color, double> *particleColorPalette;

double *particlesX;
double *particlesY;
double *particlesZ;
color *particleColors;

void initRender(double _rs, double _gr);
void createParticleRing(int nParticles, double rr, double rphi, double rtheta,
	double rdr, double rdphi, double rdtheta,
	std::pair<color, double> *rparticleColorPalette);

double getRandom(double d);

#endif
