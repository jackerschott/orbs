#define _USE_MATH_DEFINES

#include <chrono>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <utility>

#include "res.hpp"
#include "rng.hpp"
namespace ker {
#include "ker/render.h"
}
#include "render.hpp"

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

namespace render {
  double rs;
  double rg;
  uint pWidth;
  uint pHeight;
  const uint bpp = 24;
  uint sPixels;
  byte *pixels;

  uint nParticles;
  particle* particles;
  probColor *particleColorPalette;
  perspectiveCamera camera;

  bool hardwAcc = false;
  cl::Platform platform;
  cl::Device device;
  cl::Context context;
  cl::Program program;
  cl::CommandQueue queue;
  cl::Kernel kernelRender;
  int clErr;

  std::thread renderThread;
  bool _isRendering;

  bool isInit = false;
  bool isHardwAccInit = false;

  void configHardwAcc();
  void _render();
  void computeFrame(uint off, uint len);

  byte* getImageData() {
    return pixels;
  }
  bool isRendering() {
    return _isRendering;
  }

  void init(double _rs, double _rg) {
    rs = _rs;
    rg = _rg;

    isInit = true;
  }
  void initHardwAcc(cl::Platform _platform, cl::Device _device) {
    platform = _platform;
    device = _device;
    
    isHardwAccInit = true;
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
      newParticles[i].pcolor = { 255, 255, 255 };
    }

    delete[] particles;
    particles = newParticles;
    nParticles += rnParticles;
  }
  void clearParticleRings()
  {
    delete[] particles;
    particles = new particle[0];
    nParticles = 0;
  }
  void setCamera(perspectiveCamera _camera) {
    _camera.lookDir = _camera.lookDir / _camera.lookDir.getLength();
    _camera.upDir = _camera.upDir / _camera.upDir.getLength();
    camera = _camera;
  }
  void config(uint _pWidth, uint _pHeight, bool _hardwAcc) {
    pWidth = _pWidth;
    pHeight = _pHeight;

    sPixels = pWidth * pHeight * bpp / 8;
    pixels = new byte[sPixels];

    hardwAcc = _hardwAcc;
    if (hardwAcc)
      configHardwAcc();
  }
  void configHardwAcc() {
    if (!isHardwAccInit)
      throw "Hardware acceleration for rendering is not initialized.";
    std::string log;
    std::ifstream kerOptsFile(kerOptsPath);
    std::string kerOpts(std::istreambuf_iterator<char>(kerOptsFile), (std::istreambuf_iterator<char>()));
    std::ifstream kernelFile(kerPath);
    std::string src(std::istreambuf_iterator<char>(kernelFile), (std::istreambuf_iterator<char>()));

    cl::Program::Sources sources(1);
    sources[0] = std::pair<const char*, uint>(src.c_str(), src.length());

    context = cl::Context(device);
    program = cl::Program(context, sources);
    clErr = program.build(kerOpts.c_str());
    program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &log);
    if (strcmp(log.c_str(), "\n") != 0)
      std::cout << log << std::endl;
    if (clErr != 0)
      throw clErr;

    kernelRender = cl::Kernel(program, "render", &clErr);
    if (clErr != 0)
      throw clErr;

    queue = cl::CommandQueue(context, device);
  }
  void render()
  {
    if (!isInit || (hardwAcc && !isHardwAccInit))
      throw "Rendering is not initialized.";
    if (hardwAcc && !isHardwAccInit)
      throw "Hardware acceleration for rendering is not initialized.";

    _isRendering = true;
    _render();
    /*renderThread = std::thread(_render);
    renderThread.detach();*/
  }
  void _render() {
    if (!hardwAcc) {
      for (uint i = 0; i < sPixels; i++) {
        pixels[i] = 0;
      }

      if (nParticles < 50) {
        computeFrame(0, nParticles);
        _isRendering = false;
        return;
      }

      uint conc = std::thread::hardware_concurrency();
      if (conc == 0)
        throw "Optimal thread number could not be determined.";
      uint comptsPerThread = (nParticles + conc / 2) / conc;
      std::thread* threads = new std::thread[conc - 1];
      for (uint i = 0; i < conc - 1; i++) {
        threads[i] = std::thread(computeFrame, comptsPerThread * i, comptsPerThread);
      }
      computeFrame(comptsPerThread * (conc - 1), nParticles - comptsPerThread * (conc - 1));

      for (uint i = 0; i < conc - 1; i++) {
        threads[i].join();
      }
      delete[] threads;
    }
    else {
      if (nParticles == 0) {
        for (uint i = 0; i < sPixels; i++) {
          pixels[i] = 0;
        }
        _isRendering = false;
        return;
      }
      float *pR = new float[nParticles];
      float *pTheta = new float[nParticles];
      float *pPhi = new float[nParticles];
      byte *pCR = new byte[nParticles];
      byte *pCG = new byte[nParticles];
      byte *pCB = new byte[nParticles];
      for (uint i = 0; i < nParticles; i++) {
        pR[i] = (float)particles[i].r;
        pTheta[i] = (float)particles[i].theta;
        pPhi[i] = (float)particles[i].phi;
        pCR[i] = particles[i].pcolor.r;
        pCG[i] = particles[i].pcolor.g;
        pCB[i] = particles[i].pcolor.b;
      }
      cl::Buffer pixelBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sPixels, pixels, &clErr);
      cl::Buffer pRBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), pR, &clErr);
      cl::Buffer pThetaBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), pTheta, &clErr);
      cl::Buffer pPhiBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), pPhi, &clErr);
      cl::Buffer pCRBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(byte), pCR, &clErr);
      cl::Buffer pCGBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(byte), pCG, &clErr);
      cl::Buffer pCBBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(byte), pCB, &clErr);

      clErr = kernelRender.setArg<uint>(0, pWidth);
      clErr = kernelRender.setArg<uint>(1, pHeight);
      clErr = kernelRender.setArg<uint>(2, bpp);
      clErr = kernelRender.setArg<uint>(3, sPixels);
      clErr = kernelRender.setArg<cl::Buffer>(4, pixelBuf);
      clErr = kernelRender.setArg<float>(5, (float)rs);
      clErr = kernelRender.setArg<cl::Buffer>(6, pRBuf);
      clErr = kernelRender.setArg<cl::Buffer>(7, pThetaBuf);
      clErr = kernelRender.setArg<cl::Buffer>(8, pPhiBuf);
      clErr = kernelRender.setArg<cl::Buffer>(9, pCRBuf);
      clErr = kernelRender.setArg<cl::Buffer>(10, pCGBuf);
      clErr = kernelRender.setArg<cl::Buffer>(11, pCBBuf);
      clErr = kernelRender.setArg<float>(12, (float)camera.pos.x);
      clErr = kernelRender.setArg<float>(13, (float)camera.pos.y);
      clErr = kernelRender.setArg<float>(14, (float)camera.pos.z);
      clErr = kernelRender.setArg<float>(15, (float)camera.lookDir.x);
      clErr = kernelRender.setArg<float>(16, (float)camera.lookDir.y);
      clErr = kernelRender.setArg<float>(17, (float)camera.lookDir.z);
      clErr = kernelRender.setArg<float>(18, (float)camera.upDir.x);
      clErr = kernelRender.setArg<float>(19, (float)camera.upDir.y);
      clErr = kernelRender.setArg<float>(20, (float)camera.upDir.z);
      clErr = kernelRender.setArg<float>(21, (float)camera.fov);

      clErr = queue.enqueueNDRangeKernel(kernelRender, cl::NullRange, cl::NDRange(nParticles));
      if (clErr != 0)
        throw clErr;

      clErr = queue.enqueueReadBuffer(pixelBuf, true, 0, sPixels, pixels);
      if (clErr != 0)
        throw clErr;
      cl::finish();

      delete[] pR;
      delete[] pTheta;
      delete[] pPhi;
      delete[] pCR;
      delete[] pCG;
      delete[] pCB;
    }

    _isRendering = false;
  }
  void computeFrame(uint off, uint len) {
    for (uint i = off; i < len + off; i++) {
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
      if (p >= 0 && p < sPixels) {
        pixels[p] = particles[i].pcolor.r;
        pixels[p + 1] = particles[i].pcolor.g;
        pixels[p + 2] = particles[i].pcolor.b;
      }
    }
  }
  void close() {
    delete[] particles;
    delete[] pixels;
  }
}
