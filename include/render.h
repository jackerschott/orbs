#ifndef RENDER_H
#define RENDER_H

#define _USE_MATH_DEFINES

#include <chrono>
#include <climits>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <utility>

typedef unsigned char byte;
typedef unsigned int uint;

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
struct vector {
  double x;
  double y;
  double z;

  double getLength();
  static vector crossProduct(vector v1, vector v2);
  static double dotProduct(vector v1, vector v2);

  vector operator +(vector v);
  vector operator -(vector v);
  vector operator *(double d);
  vector operator /(double d);
  vector operator -();
};
struct perspectiveCamera {
  vector pos;
  vector lookDir;
  vector upDir;
  double fov;
};

static double rs;
static double rg;
static uint pWidth;
static uint pHeight;
const int bpp = 24;

void initRender(double _rs, double _rg);
void createParticleRing(uint nParticles, double rr, vector rn,
  double rdr, double rdangle,
  uint nColors, std::pair<color, double>* rparticleColorPalette);
void setCamera(perspectiveCamera _camera);
void renderConfig(uint _pWidth, uint _pHeight);
byte* render();

#endif
