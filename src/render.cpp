#define _USE_MATH_DEFINES
#define PI float(M_PI)
#define PI_2 float(M_PI_2)

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

#include "clWrapper/clwrap.hpp"
#include "glWrapper/glwrap.hpp"
#include "rng.hpp"
#include "render.hpp"

enum renderConfig {
  CONFIG_CLOSED = 0b000001,
  CONFIG_INIT = 0b000010,
  CONFIG_HAS_CAMERA = 0b000100,
  CONFIG_HAS_BG_TEX = 0b001000,
  CONFIG_RENDERING = 0b010000,

  CONFIG_INIT_FOR_RENDER = 0b001110
};

namespace render {
  // Physics data
  float rs;
  uint nParticles;
  float* ptR;
  float* ptTheta;
  float* ptPhi;
  color* ptColor;
  camera observer;
  bool isRel = false;

  // Background texture data
  uint bgWidth;
  uint bgHeight;
  uint bgBpp;
  uslong bgSImageData;
  byte *bgImageData;

  // GPU Rendering programs
  GLuint bgRenderProg;
  GLuint bgVertShader;
  GLuint bgFragShader;

  GLuint ptRenderProg;
  GLuint ptVertShader;
  GLuint ptFragShader;

  // GPU buffers
  GLuint glBackground;
  GLuint bgTexture;
  GLuint bgPosBuf;
  GLuint bgTexCoordBuf;

  GLuint glParticles;
  GLuint ptPosBuf;
  GLuint ptColorBuf;

  // Render output computation data
  const glm::vec3 bgPos[] = {
    { -1.0f, -1.0f,  0.0f },
    {  1.0f, -1.0f,  0.0f },
    {  1.0f,  1.0f,  0.0f },
    { -1.0f,  1.0f,  0.0f }
  };
  glm::vec2 bgTexCoord[4];
  glm::mat4 perspv;
  GLint viewProjn;

  // GPU computation programs
  cl::Program clglProgram;
  cl::Context clglContext;
  cl::CommandQueue clglQueue;

  cl::Kernel kernelGetPtPositions;

  // Organisation Details
  renderConfig config = CONFIG_CLOSED;

  bool isInit() {
    return (config & CONFIG_INIT) != 0;
  }
  bool isRendering() {
    return (config & CONFIG_RENDERING) != 0;
  }
  bool isClosed() {
    return (config & CONFIG_CLOSED) != 0;
  }
  void setObserverCameraAspect(float aspect) {
    assert((config & CONFIG_INIT) != 0 && (config & CONFIG_HAS_CAMERA) != 0 && (config & CONFIG_HAS_BG_TEX) != 0);
    observer.aspect = aspect;

    // Set new view projection matrix
    perspv = glm::perspective(observer.fov, observer.aspect, observer.zNear, observer.zFar);
    glm::mat4 vp = perspv * glm::lookAt(observer.pos, observer.pos + observer.lookDir, observer.upDir);

    glUseProgram(ptRenderProg);
    glUniformMatrix4fv(viewProjn, 1, false, &vp[0][0]);
    glUseProgram(0);
  }

  void init(float _rs, cl::Device device, cl::Context context) {
    assert(config == CONFIG_CLOSED);
    rs = _rs;

    // Initialize OpenGL
    glewInit();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build rendering programs
    std::string errLog;
    if (!gl::createProgram(PT_VERTEX_SHADER_SRC_PATH, PT_FRAGMENT_SHADER_SRC_PATH,
      glPtShaderInNames, &ptRenderProg, &ptVertShader, &ptFragShader, &errLog)){
      std::cerr << errLog << std::endl;
    }
    if (!gl::createProgram(BG_VERTEX_SHADER_SRC_PATH, BG_FRAGMENT_SHADER_SRC_PATH,
      glBgShaderInNames, &bgRenderProg, &bgVertShader, &bgFragShader, &errLog)) {
      std::cerr << errLog << std::endl;
    }

    // Initialize Particle Buffers
    glGenVertexArrays(1, &glParticles);
    glBindVertexArray(glParticles);

    glGenBuffers(1, &ptPosBuf);
    glBindBuffer(GL_ARRAY_BUFFER, ptPosBuf);
    glEnableVertexAttribArray(PT_POS);
    glVertexAttribPointer(PT_POS, 4, GL_FLOAT, false, 0, (void*)0);

    glGenBuffers(1, &ptColorBuf);
    glBindBuffer(GL_ARRAY_BUFFER, ptColorBuf);
    glEnableVertexAttribArray(PT_COLOR);
    glVertexAttribPointer(PT_COLOR, 4, GL_FLOAT, false, 0, (void*)0);

    glBindVertexArray(0);

    glUseProgram(ptRenderProg);
    viewProjn = glGetUniformLocation(ptRenderProg, glPtShaderUniNames[PT_VIEW_PROJ]);
    glUseProgram(0);

    // Initialize Background rendering Buffers
    glGenVertexArrays(1, &glBackground);
    glBindVertexArray(glBackground);

    glGenBuffers(1, &bgPosBuf);
    glBindBuffer(GL_ARRAY_BUFFER, bgPosBuf);
    glEnableVertexAttribArray(BG_POS);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);

    glGenBuffers(1, &bgTexCoordBuf);
    glBindBuffer(GL_ARRAY_BUFFER, bgTexCoordBuf);
    glEnableVertexAttribArray(BG_TEX_COORD);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, (void*)0);

    glBindVertexArray(0);

    // Initialize background texture
    glGenTextures(1, &bgTexture);
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Initialize GPU computation programs
    int clErr;
    std::string clSource;
    clglContext = context;

    loadFile(RENDER_KERNEL_SRC_PATH, clSource);
    clglProgram = cl::Program(clglContext, clSource);
    clErr = clglProgram.build("-cl-std=CL1.2");
    if (clErr != 0) {
      std::cerr << "Program build error " << clErr << ": " << clglProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
      throw clErr;
    }
    clglQueue = cl::CommandQueue(clglContext, device);


    config = CONFIG_INIT;
  }
  void createParticleRing(uint rnParticles, float rr, vector rn,
    float rdr, float rdtheta, float rdphi,
    uint nColors, colorBlur* rPtColorPalette) {
    assert((config & CONFIG_INIT) != 0);

    // Append new particle spherical position and color data
    float* newPtR = new float[nParticles + rnParticles];
    float* newPtTheta = new float[nParticles + rnParticles];
    float* newPtPhi = new float[nParticles + rnParticles];
    color* newPtColor = new color[nParticles + rnParticles];
    for (uslong i = 0; i < nParticles; i++) {
      newPtR[i] = ptR[i];
      newPtTheta[i] = ptTheta[i];
      newPtPhi[i] = ptPhi[i];
      newPtColor[i] = ptColor[i];
    }
    delete[] ptR;
    delete[] ptTheta;
    delete[] ptPhi;
    delete[] ptColor;
    uint maxProgLength = 0;
    std::string progress = "";
    for (uslong i = nParticles; i < nParticles + rnParticles; i++) {
      newPtR[i] = rr + normPdf(rdr / 2.0f);
      if (rn.z == 0.0) {
        newPtTheta[i] = randFloat(PI);
        if (rn.y == 0.0) {
          newPtPhi[i] = normPdf(rdphi / 2.0f) + randInt(0, 1) * PI;
        }
        else {
          newPtPhi[i] = normPdf(rdphi / 2.0f) - atan(rn.x / rn.y) + PI_2 + randInt(0, 1) * PI;
        }
      }
      else {
        newPtPhi[i] = randFloat(2.0f * PI);
        if (rn.x * cos(newPtPhi[i]) + rn.y * sin(newPtPhi[i]) == 0.0f) {
          newPtTheta[i] = normPdf(rdtheta / 2.0f) + PI_2;
        }
        else {
          newPtTheta[i] = normPdf(rdtheta / 2.0f) - atan(rn.z / (rn.x * cos(newPtPhi[i]) + rn.y * sin(newPtPhi[i]))) + PI;
          if (newPtTheta[i] > PI) {
            newPtTheta[i] -= PI;
          }
        }
      }
      color c = selectObject<color>(nColors, reinterpret_cast<std::pair<color, float>*>(rPtColorPalette));
      float fac = randFloat(0.7f, 1.0f);
      float r = (normPdf(0.03f) + c.r) * fac;
      float g = (normPdf(0.03f) + c.g) * fac;
      float b = (normPdf(0.03f) + c.b) * fac;
      float a = normPdf(0.03f) + c.a;
      newPtColor[i] = {
        r > 1.0f ? 1.0f : r,
        g > 1.0f ? 1.0f : g,
        b > 1.0f ? 1.0f : b,
        a > 1.0f ? 1.0f : a
      };

      progress = "Calculating particle positions: " + std::to_string(100 * i / (rnParticles - 1)) + " %";
      if (progress.length() > maxProgLength)
        maxProgLength = progress.length();
      else if (progress.length() < maxProgLength) {
        while (progress.length() < maxProgLength) {
          progress += " ";
        }
      }
      std::cout << '\r' << progress;
    }
    std::cout << std::endl;
    ptR = newPtR;
    ptTheta = newPtTheta;
    ptPhi = newPtPhi;
    ptColor = newPtColor;
    nParticles += rnParticles;


    // Compute cartesian particle positions and set particle position and color data
    glBindVertexArray(glParticles);

    glBindBuffer(GL_ARRAY_BUFFER, ptPosBuf);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(cl_float4), nullptr, GL_STATIC_DRAW);

    glFinish();

    cl::Kernel kernelGetPtPositions(clglProgram, kernelNames[KERNEL_GET_PT_POSITIONS]);
    cl::Buffer posBuffer = cl::BufferGL(clglContext, CL_MEM_READ_WRITE, ptPosBuf);
    cl::Buffer rBuffer = cl::Buffer(clglContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), ptR);
    cl::Buffer thetaBuffer = cl::Buffer(clglContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), ptTheta);
    cl::Buffer phiBuffer = cl::Buffer(clglContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), ptPhi);

    cl::setKernelArgs(kernelGetPtPositions, posBuffer, rBuffer, thetaBuffer, phiBuffer);
    std::vector<cl::Memory> glObjects = { posBuffer };
    clglQueue.enqueueAcquireGLObjects(&glObjects);
    clglQueue.enqueueNDRangeKernel(kernelGetPtPositions, cl::NullRange, cl::NDRange(nParticles));
    clglQueue.enqueueReleaseGLObjects(&glObjects);

    cl::finish();
    glFlush();

    glBindBuffer(GL_ARRAY_BUFFER, ptColorBuf);
    glBufferData(GL_ARRAY_BUFFER, (int)nParticles * sizeof(color), ptColor, GL_STATIC_DRAW);

    glBindVertexArray(0);
  }
  void clearParticleRings()
  {
    // Clear particle data
    assert((config & CONFIG_INIT) != 0);
    delete[] ptR;
    delete[] ptTheta;
    delete[] ptPhi;
    delete[] ptColor;

    ptR = new float[0];
    ptTheta = new float[0];
    ptPhi = new float[0];
    ptColor = new color[0];
    nParticles = 0;

    // Clear particle GPU buffers
    glBindVertexArray(glParticles);

    glBindBuffer(GL_ARRAY_BUFFER, ptPosBuf);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(cl_float4), nullptr, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ptColorBuf);
    glBufferData(GL_ARRAY_BUFFER, (int)nParticles * sizeof(color), ptColor, GL_STATIC_DRAW);

    glBindVertexArray(0);
  }
  GLuint getParticlePosBuf(uint nParticles) {
    glBindVertexArray(glParticles);

    glBindBuffer(GL_ARRAY_BUFFER, ptPosBuf);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(cl_float4), nullptr, GL_STATIC_DRAW);

    glBindVertexArray(0);

    return ptPosBuf;
  }
  GLuint getParticleColorBuf(uint nParticles) {
    glBindVertexArray(glParticles);

    glBindBuffer(GL_ARRAY_BUFFER, ptColorBuf);
    glBufferData(GL_ARRAY_BUFFER, (int)nParticles * sizeof(color), ptColor, GL_STATIC_DRAW);

    glBindVertexArray(0);

    return ptColorBuf;
  }
  void setObserverCamera(camera _observer) {
    assert((config & CONFIG_INIT) != 0 && (config & CONFIG_HAS_CAMERA) == 0);

    // Normalize Vectors and set observer camera
    _observer.lookDir = glm::normalize(_observer.lookDir);
    _observer.upDir = glm::normalize(_observer.upDir);
    observer = _observer;

    // Set View Projection Matrix
    perspv = glm::perspective(observer.fov, observer.aspect, observer.zNear, observer.zFar);
    glm::mat4 vp = perspv * glm::lookAt(observer.pos, observer.pos + observer.lookDir, observer.upDir);
    glUseProgram(ptRenderProg);
    glUniformMatrix4fv(viewProjn, 1, false, &vp[0][0]);
    glUseProgram(0);

    config = (renderConfig)(config | CONFIG_HAS_CAMERA);
  }
  void setBackgroundTex(uint sData, byte* data, uint width, uint height, uint bpp) {
    assert((config & CONFIG_INIT) != 0 && (config & CONFIG_HAS_BG_TEX) == 0);
    bgSImageData = sData;
    bgWidth = width;
    bgHeight = height;
    bgBpp = bpp;
    bgImageData = data;

    // Set background shader inputs
    glBindVertexArray(glBackground);

    glBindBuffer(GL_ARRAY_BUFFER, bgPosBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgPos), bgPos, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bgTexCoordBuf);

    glBindVertexArray(0);

    // Set background texture to passed image
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bgWidth, bgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bgImageData);

    config = (renderConfig)(config | CONFIG_HAS_BG_TEX);
  }
  void moveObserverCamera(vector pos, vector lookDir, vector upDir) {
    assert((config & CONFIG_INIT) != 0 && (config & CONFIG_HAS_CAMERA) != 0 && (config & CONFIG_HAS_BG_TEX) != 0);

    // Set new camera position
    observer.pos = pos;
    observer.lookDir = glm::normalize(lookDir);
    observer.upDir = glm::normalize(upDir);

    // Set new view projection matrix
    glm::mat4 vp = perspv * glm::lookAt(observer.pos, observer.pos + observer.lookDir, observer.upDir);

    glUseProgram(ptRenderProg);
    glUniformMatrix4fv(viewProjn, 1, false, &vp[0][0]);
    glUseProgram(0);
  }
  void renderClassic() {
    assert(config == CONFIG_INIT_FOR_RENDER);
    config = (renderConfig)(config | CONFIG_RENDERING);

    // Compute Background Texture Coordinates
    float fovY2 = 0.5f * observer.fov;
    float fovX2 = atan(observer.aspect * tan(fovY2));
    float phi1 = atan2(observer.lookDir.y, observer.lookDir.x) - fovX2;
    float phi2 = phi1 + 2.0f * fovX2;
    float theta1 = acos(observer.lookDir.z / observer.lookDir.length()) - fovY2;
    float theta2 = theta1 + 2.0f * fovY2;

    bgTexCoord[0] = { 1.0 - phi2 / (2.0f * M_PI), theta1 / M_PI };
    bgTexCoord[1] = { 1.0 - phi1 / (2.0f * M_PI), theta1 / M_PI };
    bgTexCoord[2] = { 1.0 - phi1 / (2.0f * M_PI), theta2 / M_PI };
    bgTexCoord[3] = { 1.0 - phi2 / (2.0f * M_PI), theta2 / M_PI };

    glUseProgram(bgRenderProg);
    glBindVertexArray(glBackground);
    // Set Background Texture Coordinates
    glBindBuffer(GL_ARRAY_BUFFER, bgTexCoordBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgTexCoord), bgTexCoord, GL_STATIC_DRAW);
    // Bind Background Texture
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    // Render Background
    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);

    // Render Particles
    glUseProgram(ptRenderProg);
    glBindVertexArray(glParticles);
    glDrawArrays(GL_POINTS, 0, (int)nParticles);
    glBindVertexArray(0);
    glUseProgram(0);

    config = (renderConfig)(config & ~CONFIG_RENDERING);
  }
  void renderRelativistic() {
    assert(config == CONFIG_INIT_FOR_RENDER);
    config = (renderConfig)(config | CONFIG_RENDERING);



    config = (renderConfig)(config & ~CONFIG_RENDERING);
  }
  void close() {
    assert((config & CONFIG_INIT) != 0);

    glDeleteBuffers(1, &ptPosBuf);
    glDeleteVertexArrays(1, &glParticles);

    glDetachShader(ptRenderProg, ptVertShader);
    glDeleteShader(ptVertShader);
    glDetachShader(ptRenderProg, ptFragShader);
    glDeleteShader(ptFragShader);
    glDeleteProgram(ptRenderProg);

    delete[] ptR;
    delete[] ptTheta;
    delete[] ptPhi;
    delete[] ptColor;

    delete[] bgImageData;

    config = CONFIG_CLOSED;
  }
}
