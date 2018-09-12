#ifndef RENDER_HPP
#define RENDER_HPP

#include <CL/cl.hpp>

#include "objects3d.hpp"

typedef unsigned char byte;
typedef unsigned int uint;

namespace render {
  extern double rs;
  extern double rg;
  extern uint pWidth;
  extern uint pHeight;
  extern const uint bpp;
  extern uint sPixels;

  byte* getImageData();
  bool isRendering();

  void init(double _rs, double _rg);
  void initHardwAcc(cl::Platform _platform, cl::Device _device);
  void createParticleRing(uint nParticles, double rr, vector rn,
    double rdr, double rdtheta, double rdphi,
    uint nColors, probColor* rparticleColorPalette);
  void clearParticleRings();
  void setCamera(perspectiveCamera _camera);
  void config(uint _pWidth, uint _pHeight, bool _hardwAcc);
  void render();
  void close();
}

#endif
