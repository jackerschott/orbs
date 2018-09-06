#include "render.h"

uint nParticles;
particle* particles;
probColor *particleColorPalette;
perspectiveCamera camera;

int refIndex = 0;

uint randInt(uint max);
int randInt(int min, int max);
double randDouble(double max);
double randDouble(double min, double max);
double normPdf(double d);
template<typename T> T selectObject(uint nObjects, std::pair<T, double> *collection);

double vector::getLength() {
  return sqrt(x * x + y * y + z * z);
}
vector vector::crossProduct(vector v1, vector v2) {
  return { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };
}
double vector::dotProduct(vector v1, vector v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vector vector::operator+(vector v) {
  return { x + v.x, y + v.y, z + v.z };
}
vector vector::operator-(vector v) {
  return { x - v.x, y - v.y, z - v.z };
}
vector vector::operator*(double d) {
  return { x * d, y * d, z * d };
}
vector vector::operator/(double d) {
  return { x / d, y / d, z / d };
}
vector vector::operator-() {
  return { -x, -y, -z };
}

void renderInit(double _rs, double _rg) {
  rs = _rs;
  rg = _rg;
}
void createParticleRing(uint rnParticles, double rr, vector rn, 
  double rdr, double rdtheta, double rdphi,
  uint nColors, probColor* rparticleColorPalette) {

  particle* newParticles = new particle[nParticles + rnParticles];
  for (uint i = 0; i < nParticles; i++) {
    newParticles[i] = particles[i];
  }
  for (uint i = nParticles; i < nParticles + rnParticles; i++) {
    newParticles[i].r = rr + normPdf(rdr / 2.0);
    if (rn.z == 0.0) {
      newParticles[i].theta = randDouble(M_PI);
      if (rn.y == 0.0) {
        newParticles[i].phi = normPdf(rdphi / 2.0) + randInt(0, 1) * M_PI;
      }
      else {
        newParticles[i].phi = normPdf(rdphi / 2.0) - atan(rn.x / rn.y) + M_PI_2 + randInt(0, 1) * M_PI;
      }
    }
    else {
      newParticles[i].phi = randDouble(2.0 * M_PI);
      if (rn.x * cos(newParticles[i].phi) + rn.y * sin(newParticles[i].phi) == 0.0) {
        newParticles[i].theta = normPdf(rdtheta / 2.0) + M_PI_2;
      }
      else {
        newParticles[i].theta = normPdf(rdtheta / 2.0) - atan(rn.z / (rn.x * cos(newParticles[i].phi) + rn.y * sin(newParticles[i].phi))) + M_PI;
        if (newParticles[i].theta > M_PI) {
          newParticles[i].theta -= M_PI;
        }
      }
    }
    newParticles[i].vr = 0.0;
    newParticles[i].vphi = 1.0;
    newParticles[i].vtheta = 0.0;
    newParticles[i].pcolor = {255, 255, 255};
  }

  delete[] particles;
  particles = newParticles;
  nParticles += rnParticles;
}
void setCamera(perspectiveCamera _camera) {
  _camera.lookDir = _camera.lookDir / _camera.lookDir.getLength();
  _camera.upDir = _camera.upDir / _camera.upDir.getLength();
  camera = _camera;
}
void renderConfig(uint _pWidth, uint _pHeight) {
  pWidth = _pWidth;
  pHeight = _pHeight;
}
byte* render()
{
  uint nPixelBytes = pWidth * pHeight * bpp / 8;
  byte* pixels = new byte[nPixelBytes];
  for (uint i = 0; i < nPixelBytes; i++) {
    pixels[i] = 0;
  }
  for (uint i = 0; i < nParticles; i++) {

    vector particlePos;
    particlePos.x = particles[i].r * sin(particles[i].theta) * cos(particles[i].phi);
    particlePos.y = particles[i].r * sin(particles[i].theta) * sin(particles[i].phi);
    particlePos.z = particles[i].r * cos(particles[i].theta);

    vector screenPoint;
    screenPoint = camera.pos + (particlePos - camera.pos) * vector::dotProduct(camera.lookDir, camera.lookDir)
      / vector::dotProduct(camera.lookDir, particlePos - camera.pos);
    vector xDir = vector::crossProduct(camera.lookDir, camera.upDir);
    vector screenX = xDir / pWidth;
    vector screenY = vector::crossProduct(camera.lookDir, xDir) / pWidth;
    double cameraX = vector::dotProduct(screenPoint, screenX) / vector::dotProduct(screenX, screenX);
    double cameraY = vector::dotProduct(screenPoint, screenY) / vector::dotProduct(screenY, screenY);

    int pixelX = (int)(cameraX + pWidth / 2.0);
    int pixelY = (int)(cameraY + pHeight / 2.0);
    uint p = (pixelX + pWidth * pixelY) * bpp / 8;
    if (p >= 0 && p < nPixelBytes) {
      pixels[p] = particles[i].pcolor.r;
      pixels[p + 1] = particles[i].pcolor.g;
      pixels[p + 2] = particles[i].pcolor.b;
    }
  }
  return pixels;
}

uint rng()
{
  static uint counter = 0;
  randutils::seed_seq_fe256 seeder{ counter,0u,0u,0u };
  ++counter;
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
double randDouble() {
  return (double)rng() / UINT_MAX;
}
double randDouble(double max) {
  return (double)rng() / UINT_MAX * max;
}
double randDouble(double min, double max) {
  return (double)rng() / UINT_MAX * (max - min) + min;
}
double normPdf(double d) {
  std::default_random_engine generator;
  generator.seed(rng());
  std::normal_distribution<double> distribution(0, d);
  return distribution(generator);
}
template<typename T> T selectObject(uint nObjects, std::pair<T, double> *collection) {
  std::default_random_engine generator;
  generator.seed(rng());
  std::uniform_int_distribution<uint> distribution(0, UINT_MAX - 1);
  uint selNumber = distribution(generator);
  uint probLimit = 0;
  for (uint i = 0; i < nObjects; i++) {
    probLimit += UINT_MAX * collection[i].second;
    if (selNumber < probLimit) {
      return collection[i].first;
    }
  }
  throw "test";
}
