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

  uint partPxSize;
  int *screenX;
  int *screenY;
  cl::Buffer screenRelXBuf;
  cl::Buffer screenRelYBuf;
  cl::Buffer screenRelFacBuf;
  cl::Buffer pRBuf;
  cl::Buffer pThetaBuf;
  cl::Buffer pPhiBuf;
  cl::Buffer pCRBuf;
  cl::Buffer pCGBuf;
  cl::Buffer pCBBuf;

  bool hardwAcc = false;
  cl::Platform platform;
  cl::Device device;
  cl::Context context;
  cl::Program program;
  cl::CommandQueue queue;
  cl::Kernel kerRenderScreenPos;
  cl::Kernel kerRenderShapes;
  int clErr;

  std::thread renderThread;
  bool _isRendering;

  bool isInit = false;
  bool isHardwAccInit = false;

  void onChangeParticleNumber();
  void onChangeParticlePos();
  void _render();
  void computeFrame(uint off, uint len);
  float m(float x, float y, float s);

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

    std::string log;
    std::ifstream kerOptsFile(kerOptsPath);
    std::string kerOpts(std::istreambuf_iterator<char>(kerOptsFile), (std::istreambuf_iterator<char>()));
    std::ifstream kernelFile(kerPath);
    std::string src(std::istreambuf_iterator<char>(kernelFile), (std::istreambuf_iterator<char>()));

    cl::Program::Sources sources(1);
    sources[0] = std::pair<const char*, uint>(src.c_str(), (uint)src.length());

    context = cl::Context(device);
    program = cl::Program(context, sources, &clErr);
    clErr = program.build(kerOpts.c_str());
    program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &log);
    if (strcmp(log.c_str(), "\n") != 0)
      std::cout << log << std::endl;
    if (clErr != 0)
      throw clErr;

    kerRenderScreenPos = cl::Kernel(program, kerNames[0], &clErr);
    if (clErr != 0)
      throw clErr;
    kerRenderShapes = cl::Kernel(program, kerNames[1], &clErr);
    if (clErr != 0)
      throw clErr;

    queue = cl::CommandQueue(context, device);
    
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

    onChangeParticleNumber();
  }
  void clearParticleRings()
  {
    delete[] particles;
    particles = new particle[0];
    nParticles = 0;

    onChangeParticleNumber();
  }
  void onChangeParticleNumber() {
    delete[] screenX;
    delete[] screenY;
    screenX = new int[nParticles];
    screenY = new int[nParticles];
    byte *pCR = new byte[nParticles];
    byte *pCG = new byte[nParticles];
    byte *pCB = new byte[nParticles];
    for (uint i = 0; i < nParticles; i++) {
      pCR[i] = particles[i].pcolor.r;
      pCG[i] = particles[i].pcolor.g;
      pCB[i] = particles[i].pcolor.b;
    }
    pCRBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(byte), pCR, &clErr);
    pCGBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(byte), pCG, &clErr);
    pCBBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(byte), pCB, &clErr);
    delete[] pCR;
    delete[] pCG;
    delete[] pCB;

    onChangeParticlePos();
  }
  void onChangeParticlePos() {
    float *pR = new float[nParticles];
    float *pTheta = new float[nParticles];
    float *pPhi = new float[nParticles];
    for (uint i = 0; i < nParticles; i++) {
      pR[i] = (float)particles[i].r;
      pTheta[i] = (float)particles[i].theta;
      pPhi[i] = (float)particles[i].phi;
    }
    pRBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), pR, &clErr);
    pThetaBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), pTheta, &clErr);
    pPhiBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), pPhi, &clErr);
    delete[] pR;
    delete[] pTheta;
    delete[] pPhi;
  }
  void setCamera(perspectiveCamera _camera) {
    _camera.lookDir = _camera.lookDir / _camera.lookDir.getLength();
    _camera.upDir = _camera.upDir / _camera.upDir.getLength();
    camera = _camera;
  }
  void config(uint _pWidth, uint _pHeight, uint partRad, bool _hardwAcc) {
    pWidth = _pWidth;
    pHeight = _pHeight;
    hardwAcc = _hardwAcc;

    sPixels = pWidth * pHeight * bpp / 8;
    pixels = new byte[sPixels];

    std::vector<int> vScreenRelX;
    std::vector<int> vScreenRelY;
    std::vector<float> vScreenRelFac;
#define SET_PIXEL(x, y) vScreenRelX.push_back(x); vScreenRelY.push_back(y); vScreenRelFac.push_back(m((float)x, (float)y, (float)r));

    int r = partRad;//(pWidth + pHeight) / (2 * sizeToRad);
    int x = r;
    int y = 0;
    int err = r;
    for (int xi = -x; xi < x + 1; ++xi) {
      SET_PIXEL(xi, y);
    }
    for (int xi = -y; xi < y; ++xi) {
      SET_PIXEL(y, x);
      SET_PIXEL(y, -x);
    }
    while (y < x) {
      err -= (y << 1) + 1;
      ++y;
      if (err < 0) {
        err += (x << 1) - 1;
        --x;

        if (y < x) {
          for (int xi = -y; xi < y + 1; ++xi) {
            SET_PIXEL(xi, x);
            SET_PIXEL(xi, -x);
          }
        }
      }
      else {
        SET_PIXEL(y, x);
        SET_PIXEL(-y, x);
        SET_PIXEL(y, -x);
        SET_PIXEL(-y, -x);
      }

      for (int xi = -x; xi < x + 1; ++xi) {
        SET_PIXEL(xi, y);
        SET_PIXEL(xi, -y);
      }
    }

    partPxSize = (uint)vScreenRelX.size();
    int *screenRelX = new int[partPxSize];
    int *screenRelY = new int[partPxSize];
    float* screenRelFac = new float[partPxSize];
    for (uint i = 0; i < partPxSize; i++) {
      screenRelX[i] = vScreenRelX[i];
      screenRelY[i] = vScreenRelY[i];
      screenRelFac[i] = vScreenRelFac[i];
    }
    screenRelXBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, partPxSize * sizeof(int), screenRelX, &clErr);
    screenRelYBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, partPxSize * sizeof(int), screenRelY, &clErr);
    screenRelFacBuf = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, partPxSize * sizeof(float), screenRelFac, &clErr);
    delete[] screenRelX;
    delete[] screenRelY;
    delete[] screenRelFac;
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
      cl::Buffer pixelBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sPixels, pixels, &clErr);
      cl::Buffer screenXBuf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, nParticles * sizeof(int), screenX, &clErr);
      cl::Buffer screenYBuf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, nParticles * sizeof(int), screenY, &clErr);

      clErr = kerRenderScreenPos.setArg<uint>(0, pWidth);
      clErr = kerRenderScreenPos.setArg<uint>(1, pHeight);
      clErr = kerRenderScreenPos.setArg<uint>(2, bpp);
      clErr = kerRenderScreenPos.setArg<cl::Buffer>(3, screenXBuf);
      clErr = kerRenderScreenPos.setArg<cl::Buffer>(4, screenYBuf);
      clErr = kerRenderScreenPos.setArg<float>(5, (float)rs);
      clErr = kerRenderScreenPos.setArg<cl::Buffer>(6, pRBuf);
      clErr = kerRenderScreenPos.setArg<cl::Buffer>(7, pThetaBuf);
      clErr = kerRenderScreenPos.setArg<cl::Buffer>(8, pPhiBuf);
      clErr = kerRenderScreenPos.setArg<float>(9, (float)camera.pos.x);
      clErr = kerRenderScreenPos.setArg<float>(10, (float)camera.pos.y);
      clErr = kerRenderScreenPos.setArg<float>(11, (float)camera.pos.z);
      clErr = kerRenderScreenPos.setArg<float>(12, (float)camera.lookDir.x);
      clErr = kerRenderScreenPos.setArg<float>(13, (float)camera.lookDir.y);
      clErr = kerRenderScreenPos.setArg<float>(14, (float)camera.lookDir.z);
      clErr = kerRenderScreenPos.setArg<float>(15, (float)camera.upDir.x);
      clErr = kerRenderScreenPos.setArg<float>(16, (float)camera.upDir.y);
      clErr = kerRenderScreenPos.setArg<float>(17, (float)camera.upDir.z);
      clErr = kerRenderScreenPos.setArg<float>(18, (float)camera.fov);

      clErr = kerRenderShapes.setArg<uint>(0, pWidth);
      clErr = kerRenderShapes.setArg<uint>(1, pHeight);
      clErr = kerRenderShapes.setArg<uint>(2, bpp);
      clErr = kerRenderShapes.setArg<uint>(3, sPixels);
      clErr = kerRenderShapes.setArg<cl::Buffer>(4, pixelBuf);
      clErr = kerRenderShapes.setArg<uint>(5, partPxSize);
      clErr = kerRenderShapes.setArg<cl::Buffer>(6, screenXBuf);
      clErr = kerRenderShapes.setArg<cl::Buffer>(7, screenYBuf);
      clErr = kerRenderShapes.setArg<cl::Buffer>(8, screenRelXBuf);
      clErr = kerRenderShapes.setArg<cl::Buffer>(9, screenRelYBuf);
      clErr = kerRenderShapes.setArg<cl::Buffer>(10, screenRelFacBuf);
      clErr = kerRenderShapes.setArg<cl::Buffer>(11, pCRBuf);
      clErr = kerRenderShapes.setArg<cl::Buffer>(12, pCGBuf);
      clErr = kerRenderShapes.setArg<cl::Buffer>(13, pCBBuf);

      clErr = queue.enqueueNDRangeKernel(kerRenderScreenPos, cl::NullRange, cl::NDRange(nParticles));
      if (clErr != 0)
        throw clErr;
      clErr = queue.enqueueNDRangeKernel(kerRenderShapes, cl::NullRange, cl::NDRange(nParticles * partPxSize));
      if (clErr != 0)
        throw clErr;
      clErr = queue.enqueueReadBuffer(pixelBuf, true, 0, sPixels, pixels);
      if (clErr != 0)
        throw clErr;
      cl::finish();

      delete[] screenX;
      delete[] screenY;
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

  float m(float x, float y, float s) {
    float r2 = x * x + y * y;
    float r = sqrt(r2);
    return (2.0f * r - 3.0f * s) *  r2 / (s * s * s) + 1.0f;
  }
}
