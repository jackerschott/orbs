#ifndef RENDER_H
#define RENDER_H

#include <iostream>

typedef unsigned char byte;

struct color {
	byte r;
	byte g;
	byte b;
};

double rs;
double gr;
std::pair<color, double> *particleColorPalette;

double *particlesX;
double *particlesY;
double *particlesZ;
color *particleColors;

void initRender(double _rs, double _gr, std::pair<color, double> *_particleColorPalette);
void createParticleRing(double r, double phi, double theta, double dr, double dPhi, double dTheta);

double getRandomDouble(double lower, double upper);

#endif