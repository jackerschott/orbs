#define _USE_MATH_DEFINES
#define PI ((float)M_PI)
#define PI_2 ((float)M_PI_2)

#include <chrono>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <utility>

#include <GL/glew.h>

#include "clWrapper/clwrap.hpp"
#include "glWrapper/glwrap.hpp"
#include "rng.hpp"
#include "render.hpp"

#define USE_GL true

namespace render {
  bool isInit = false;
  bool isHardwAccInit = false;
  bool _isRendering;

  uint pWidth;
  uint pHeight;
  const uint bpp = 24;
  ulong sPixelData;
  byte *pixelData;

  uint bgWidth;
  uint bgHeight;
  uint bgBpp;
  ulong bgSPixels;
  byte *bgPixels;

  uint ptNPixels;
  int *PtProjX;
  int *PtProjY;
  int *screenRelX;
  int *screenRelY;
  float* screenRelFac;

  bool hardwAcc = false;
  uint maxSWGroup;
  cl::Platform platform;
  cl::Device device;
  cl::Context context;
  cl::Program program;
  cl::CommandQueue queue;
  int clErr;

  cl::Kernel kerRenderBg;
  cl::Buffer bufBgPixels;

  cl::Kernel kerComputePtProj;
  cl::Buffer bufPtR;
  cl::Buffer bufPtTheta;
  cl::Buffer bufPtPhi;

  cl::Kernel kerDrawPtShapes;
  cl::Buffer bufPtProjRelX;
  cl::Buffer bufPtProjRelY;
  cl::Buffer bufPtProjRelCFac;
  cl::Buffer bufPtCR;
  cl::Buffer bufPtCG;
  cl::Buffer bufPtCB;

  float rs;
  float rg;
  ulong nParticles;
  particle* particles;
  probColor *particleColorPalette;
  perspectiveCamera camera;
  bool isRelativistic = false;

  void onChangeParticleNumber();
  void onChangeParticlePos();
  void renderCla();
  void renderClaGpu();
	void renderClaGpuGL();
  void renderRel();
  void renderRelGpu();
  float m(float x, float y, float s);

  byte* getImageData() {
    return pixelData;
  }
  bool isRendering() {
    return _isRendering;
  }

  void init(float _rs, float _rg) {
    rs = _rs;
    rg = _rg;

    isInit = true;
  }
  void initHardwAcc(cl::Platform _platform, cl::Device _device) {
    platform = _platform;
    device = _device;

    for (maxSWGroup = 1; maxSWGroup <= CL_DEVICE_MAX_WORK_GROUP_SIZE; maxSWGroup <<= 1); maxSWGroup >>= 1;

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
    if (strcmp(log.c_str(), "\n") != 0) {
      std::cout << log << std::endl;
      std::cin.get();
    }
    if (clErr != 0)
      throw clErr;

    kerRenderBg = cl::Kernel(program, kerNames[0], &clErr);
    if (clErr != 0)
      throw clErr;
    kerComputePtProj = cl::Kernel(program, kerNames[1], &clErr);
    if (clErr != 0)
      throw clErr;
    kerDrawPtShapes = cl::Kernel(program, kerNames[2], &clErr);
    if (clErr != 0)
      throw clErr;

    queue = cl::CommandQueue(context, device);
    
    isHardwAccInit = true;
  }
  void createParticleRing(ulong rnParticles, float rr, vector rn,
      float rdr, float rdtheta, float rdphi,
      uint nColors, probColor* rparticleColorPalette) {

    particle* newParticles = new particle[nParticles + rnParticles];
    for (ulong i = 0; i < nParticles; i++) {
      newParticles[i] = particles[i];
    }
    for (ulong i = nParticles; i < nParticles + rnParticles; i++) {
      newParticles[i].r = rr + normPdf(rdr / 2.0f);
      if (rn.z == 0.0) {
        newParticles[i].theta = randDouble(PI);
        if (rn.y == 0.0) {
          newParticles[i].phi = normPdf(rdphi / 2.0f) + randInt(0, 1) * PI;
        }
        else {
          newParticles[i].phi = normPdf(rdphi / 2.0f) - atan(rn.x / rn.y) + PI_2 + randInt(0, 1) * PI;
        }
      }
      else {
        newParticles[i].phi = randDouble(2.0f * PI);
        if (rn.x * cos(newParticles[i].phi) + rn.y * sin(newParticles[i].phi) == 0.0f) {
          newParticles[i].theta = normPdf(rdtheta / 2.0f) + PI_2;
        }
        else {
          newParticles[i].theta = normPdf(rdtheta / 2.0f) - atan(rn.z / (rn.x * cos(newParticles[i].phi) + rn.y * sin(newParticles[i].phi))) + PI;
          if (newParticles[i].theta > PI) {
            newParticles[i].theta -= PI;
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
    delete[] PtProjX;
    delete[] PtProjY;
    PtProjX = new int[nParticles];
    PtProjY = new int[nParticles];
    byte *pCR = new byte[nParticles];
    byte *pCG = new byte[nParticles];
    byte *pCB = new byte[nParticles];
    for (uint i = 0; i < nParticles; i++) {
      pCR[i] = particles[i].pcolor.r;
      pCG[i] = particles[i].pcolor.g;
      pCB[i] = particles[i].pcolor.b;
    }
    bufPtCR = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(byte), pCR, &clErr);
    bufPtCG = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(byte), pCG, &clErr);
    bufPtCB = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(byte), pCB, &clErr);
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
    bufPtR = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), pR, &clErr);
    bufPtTheta = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), pTheta, &clErr);
    bufPtPhi = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), pPhi, &clErr);
    delete[] pR;
    delete[] pTheta;
    delete[] pPhi;
  }
  void setCamera(perspectiveCamera _camera) {
    _camera.lookDir = _camera.lookDir / _camera.lookDir.getLength();
    _camera.upDir = _camera.upDir / _camera.upDir.getLength();
    camera = _camera;
  }
  void setBackground(uint sData, byte* data, uint width, uint height, uint bpp) {
    bgSPixels = sData;
    bgWidth = width;
    bgHeight = height;
    bgBpp = bpp;
    bgPixels = data;

    bufBgPixels = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, bgSPixels, bgPixels, &clErr);
    if (clErr != 0) {
      std::cout << "Allocation Error" << std::endl;
      throw clErr;
    }
  }
  void config(uint _pWidth, uint _pHeight, uint partRad, bool _hardwAcc) {
    pWidth = _pWidth;
    pHeight = _pHeight;
    hardwAcc = _hardwAcc;

    sPixelData = pWidth * pHeight * bpp / 8;
    pixelData = new byte[sPixelData];

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

    ptNPixels = (uint)vScreenRelX.size();
    screenRelX = new int[ptNPixels];
    screenRelY = new int[ptNPixels];
    screenRelFac = new float[ptNPixels];
    for (uint i = 0; i < ptNPixels; i++) {
      screenRelX[i] = vScreenRelX[i];
      screenRelY[i] = vScreenRelY[i];
      screenRelFac[i] = vScreenRelFac[i];
    }
    bufPtProjRelX = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, ptNPixels * sizeof(int), screenRelX, &clErr);
    bufPtProjRelY = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, ptNPixels * sizeof(int), screenRelY, &clErr);
    bufPtProjRelCFac = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, ptNPixels * sizeof(float), screenRelFac, &clErr);
  }
  void render()
  {
    if (!isInit)
      throw "Rendering is not initialized.";
    if (hardwAcc && !isHardwAccInit)
      throw "Hardware acceleration for rendering is not initialized.";

    _isRendering = true;
    if (isRelativistic) {
      if (hardwAcc)
        renderRelGpu();
      else renderRel();
    }
    else {
			if (hardwAcc) {
#if USE_GL
				renderClaGpuGL();
#else
				renderClaCL();
#endif
			}
      else renderCla();
    }
    _isRendering = false;
  }
  void renderCla() {
    std::chrono::high_resolution_clock::time_point *t = new std::chrono::high_resolution_clock::time_point[3];
    t[0] = std::chrono::high_resolution_clock::now();

    memset(pixelData, 0, sPixelData);

    t[1] = std::chrono::high_resolution_clock::now();

    for (uint i = 0; i < nParticles; i++) {
      vector particlePos;
      particlePos.x = particles[i].r * sin(particles[i].theta) * cos(particles[i].phi);
      particlePos.y = particles[i].r * sin(particles[i].theta) * sin(particles[i].phi);
      particlePos.z = particles[i].r * cos(particles[i].theta);

      vector screenPoint;
      screenPoint = camera.pos + (particlePos - camera.pos) * vector::dotProduct(camera.lookDir, camera.lookDir)
        / vector::dotProduct(camera.lookDir, particlePos - camera.pos);
      vector xDir = vector::crossProduct(camera.lookDir, camera.upDir);
      vector screenX = xDir / (float)pWidth;
      vector screenY = vector::crossProduct(camera.lookDir, xDir) / (float)pWidth;
      float cameraX = vector::dotProduct(screenPoint, screenX) / vector::dotProduct(screenX, screenX);
      float cameraY = vector::dotProduct(screenPoint, screenY) / vector::dotProduct(screenY, screenY);

      float bhSiding = vector::dotProduct(particlePos, camera.pos - particlePos);
      float bhAlignm = bhSiding * bhSiding - vector::dotProduct(camera.pos - particlePos, camera.pos - particlePos) * (vector::dotProduct(particlePos, particlePos) - rs * rs);
      if (bhSiding <= 0.0f && bhAlignm >= 0.0f) {
        continue;
      }

      int pixelX = (int)(cameraX + pWidth / 2.0);
      int pixelY = (int)(cameraY + pHeight / 2.0);
      for (uint j = 0; j < ptNPixels; j++) {
        int x = pixelX + screenRelX[j];
        int y = pixelY + screenRelY[j];
        int p = (x + pWidth * y) * bpp / 8;
        if (p >= 0 && p < (int)sPixelData) {
#define BYTE_MAX_CUT(b) (b) <= 255.0f ? (byte)(b) : (byte)255
          pixelData[p] = BYTE_MAX_CUT(particles[i].pcolor.r * screenRelFac[j] + pixelData[p]);
          pixelData[p + 1] = BYTE_MAX_CUT(particles[i].pcolor.g * screenRelFac[j] + pixelData[p + 1]);
          pixelData[p + 2] = BYTE_MAX_CUT(particles[i].pcolor.b * screenRelFac[j] + pixelData[p + 2]);
        }
      }
    }

    t[2] = std::chrono::high_resolution_clock::now();

    double renderTime = 0.0;
    for (int i = 1; i < 4; i++) {
      double sec = std::chrono::duration_cast<std::chrono::nanoseconds>(t[i] - t[i - 1]).count() * 1.0e-9;
      renderTime += sec;
    }
    for (int i = 1; i < 4; i++) {
      double sec = std::chrono::duration_cast<std::chrono::nanoseconds>(t[i] - t[i - 1]).count() * 1.0e-9;
      std::cout << "time from " << i - 1 << " to " << i << " : " << sec << " s" << std::endl;
      std::cout << "share: " << sec / renderTime * 100.0 << " %" << std::endl;
    }
    std::cout << "overall time: " << renderTime << " s" << std::endl;
    std::cout << "frame rate: " << 1.0 / renderTime << " fps" << std::endl;
    std::cout << std::endl;
  }
  void renderClaGpu() {
		uint it = 0;
		std::chrono::high_resolution_clock::time_point *t = new std::chrono::high_resolution_clock::time_point[100];
		t[it] = std::chrono::high_resolution_clock::now(); ++it; // 0

    cl::Buffer bufPixelData(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sPixelData, pixelData, &clErr);
    cl::Buffer bufPtProjX(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, nParticles * sizeof(int), PtProjX, &clErr);
    cl::Buffer bufPtProjY(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, nParticles * sizeof(int), PtProjY, &clErr);

    if (nParticles == 0) {
      clErr = queue.enqueueReadBuffer(bufPixelData, true, 0, sPixelData, pixelData);
      _isRendering = false;
      return;
    }
		t[it] = std::chrono::high_resolution_clock::now(); ++it; // 1

		cl::setKernelArgs(kerComputePtProj, pWidth, pHeight, bpp, bufPtProjX, bufPtProjY, rs, bufPtR, bufPtTheta, bufPtPhi,
			camera.pos.x, camera.pos.y, camera.pos.z, camera.lookDir.x, camera.lookDir.y, camera.lookDir.z, camera.upDir.x, camera.upDir.y, camera.upDir.z, camera.fov);
    clErr = queue.enqueueNDRangeKernel(kerComputePtProj, cl::NullRange, cl::NDRange(nParticles));
    if (clErr != 0)
      throw clErr;
		t[it] = std::chrono::high_resolution_clock::now(); ++it; // 2

		cl::finish();
		cl::setKernelArgs(kerDrawPtShapes, pWidth, pHeight, bpp, sPixelData, bufPixelData, ptNPixels, bufPtProjX, bufPtProjY,
			bufPtProjRelX, bufPtProjRelY, bufPtProjRelCFac, bufPtCR, bufPtCG, bufPtCB);
    clErr = queue.enqueueNDRangeKernel(kerDrawPtShapes, cl::NullRange, cl::NDRange(nParticles * ptNPixels));
    if (clErr != 0)
      throw clErr;
		t[it] = std::chrono::high_resolution_clock::now(); ++it; // 3

		cl::finish();
		queue.enqueueReadBuffer(bufPixelData, true, 0, sPixelData, pixelData);
		t[it] = std::chrono::high_resolution_clock::now(); ++it; // 4

		double renderTime = 0.0;
		for (uint i = 1; i < it; i++) {
			double sec = std::chrono::duration_cast<std::chrono::nanoseconds>(t[i] - t[i - 1]).count() * 1.0e-9;
			renderTime += sec;
		}
		for (uint i = 1; i < it; i++) {
			double sec = std::chrono::duration_cast<std::chrono::nanoseconds>(t[i] - t[i - 1]).count() * 1.0e-9;
			std::cout << "time from " << i - 1 << " to " << i << " : " << sec << " s" << std::endl;
			std::cout << "share: " << sec / renderTime * 100.0 << " %" << std::endl;
		}
		std::cout << "overall time: " << renderTime << " s" << std::endl;
		std::cout << "frame rate: " << 1.0 / renderTime << " fps" << std::endl;
		std::cout << std::endl;
  }
	void renderClaGpuGL() {
		
	}
  void renderRel() {

  }
  void renderRelGpu() {

  }
  void close() {
    delete[] PtProjX;
    delete[] PtProjY;
    delete[] particles;
    delete[] pixelData;
    delete[] bgPixels;
  }

  float m(float x, float y, float s) {
    float r2 = x * x + y * y;
    float r = sqrt(r2);
    return (2.0f * r - 3.0f * s) *  r2 / (s * s * s) + 1.0f;
  }
}
