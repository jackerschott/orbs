#ifndef RENDER_HPP
#define RENDER_HPP

#include <CL/cl.hpp>
#include <glm/glm.hpp>

#include "res.hpp"

typedef glm::vec3 vector;

struct camera;
struct color24;
union color32;
struct color;
struct colorBlur;

struct camera {
  vector pos;
  vector lookDir;
  vector upDir;
  float fov;
  float aspect;
  float zNear;
  float zFar;
};
struct color24 {
  byte r;
  byte g;
  byte b;
};
union color32 {
  struct {
    byte r;
    byte g;
    byte b;
    byte a;
  };
  int data;
};
struct color {
  float r;
  float g;
  float b;
  float a;
};
struct colorBlur {
  color clr;
  float size;
};

namespace render {
  extern float rs;
  extern float rg;
  extern uint pWidth;
  extern uint pHeight;
  extern const uint bpp;
  extern ulong sPixelData;

  bool isRendering();

  void init(float _rs, float _rg, bool _hardwAcc = true);
  void initHardwAcc(cl::Device device, cl::Context context);
  void createParticleRing(uint nParticles, float rr, vector rn,
    float rdr, float rdtheta, float rdphi,
    uint nColors, colorBlur* rparticleColorPalette);
  void clearParticleRings();
  void setObserver(camera _observer);
  void setBackground(uint sData, byte* data, uint width, uint height, uint bpp);
  void render();
  void close();
}

#endif
