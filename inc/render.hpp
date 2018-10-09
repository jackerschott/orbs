#ifndef RENDER_HPP
#define RENDER_HPP

// Device: NVIDIA GeForce GTX 1080 Ti with OpenCL 1.2 CUDA
#include <CL/cl.hpp>

#include "objects3d.hpp"
#include "res.hpp"

typedef unsigned char byte;
typedef unsigned int uint;

namespace render {
  extern float rs;
  extern float rg;
  extern uint pWidth;
  extern uint pHeight;
  extern const uint bpp;
  extern ulong sPixelData;

  byte* getImageData();
  bool isRendering();

  void init(float _rs, float _rg);
  void initHardwAcc(cl::Platform _platform, cl::Device _device);
  void createParticleRing(ulong nParticles, float rr, vector rn,
    float rdr, float rdtheta, float rdphi,
    uint nColors, probColor* rparticleColorPalette);
  void clearParticleRings();
  void setCamera(perspectiveCamera _camera);
  void setBackground(uint sData, byte* data, uint width, uint height, uint bpp);
  void config(uint _pWidth, uint _pHeight, uint partRad, bool _hardwAcc);
  void render();
  void close();
}

#endif
