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

// OpenGL Version: 4.6.0 NVIDIA 397.44
// OpenGL Shading Language Version: 4.60 NVIDIA

#include "clWrapper/clwrap.hpp"
#include "glWrapper/glwrap.hpp"
#include "rng.hpp"
#include "render.hpp"

namespace render {
  // Physics Data
  float rs;
  float rg;
  uint nParticles;
  float* ptR;
  float* ptTheta;
  float* ptPhi;
  color* ptColor;
  camera observer;
  bool isRel = false;

  // Background Texture
  uint bgWidth;
  uint bgHeight;
  uint bgBpp;
  ulong bgSImageData;
  byte *bgImageData;

  // OpenGL Variables
  GLuint bgProgram;
  GLuint bgVertShader;
  GLuint bgFragShader;

  GLuint ptProgram;
  GLuint ptVertShader;
  GLuint ptFragShader;

  GLuint glBg;
  GLuint glBgTexture;
  GLuint glBgPositionBuffer;
  GLuint glBgTexCoordBuffer;
  glm::vec2 bgTexCoord[4];

  GLuint glParticles;
  GLuint glPtPositionBuf;
  GLuint ptColorBuf;

  std::chrono::high_resolution_clock::time_point t0;
  glm::mat4 persp;
  GLint glViewProj;

  // OpenCL Variables
  cl::Program clProgram;
  cl::Context clContext;
  cl::CommandQueue clQueue;

  cl::Kernel kernelGetPtPositions;

  // Configuration Details
  bool isInit = false;
  bool isHardwAccInit = false;
  bool hardwAcc = false;
  bool _isRendering;

  void renderCla();
  void renderClaGpu();
  void renderRel();
  void renderRelGpu();

  bool isRendering() {
    return _isRendering;
  }

  void init(float _rs, float _rg, bool _hardwAcc) {
    rs = _rs;
    rg = _rg;
    hardwAcc = _hardwAcc;

    isInit = true;
  }
  void initHardwAcc(cl::Device device, cl::Context context) {
    glewInit();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::string source;
    const char* src;
    int len;

    ptVertShader = glCreateShader(GL_VERTEX_SHADER);
    loadFile(PT_VERTEX_SHADER_SRC_PATH, source);
    src = source.c_str();
    len = (int)source.length();
    glShaderSource(ptVertShader, 1, &src, &len);

    ptFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    loadFile(PT_FRAGMENT_SHADER_SRC_PATH, source);
    src = source.c_str();
    len = (int)source.length();
    glShaderSource(ptFragShader, 1, &src, &len);

    ptProgram = glCreateProgram();
    glCompileShader(ptVertShader);
    if (gl::checkForShaderErr(ptVertShader, GL_COMPILE_STATUS)) {
      std::cerr << gl::getShaderInfoLog(ptVertShader) << std::endl;
    }
    glAttachShader(ptProgram, ptVertShader);

    glCompileShader(ptFragShader);
    if (gl::checkForShaderErr(ptFragShader, GL_COMPILE_STATUS)) {
      std::cerr << gl::getShaderInfoLog(ptFragShader) << std::endl;
    }
    glAttachShader(ptProgram, ptFragShader);

    for (uint i = 0; i < NUM_PT_SHADER_ATTR; i++) {
      glBindAttribLocation(ptProgram, i, glPtShaderIns[i]);
    }
    glLinkProgram(ptProgram);
    if (gl::checkForProgramErr(ptProgram, GL_LINK_STATUS)) {
      std::cerr << gl::getProgramInfoLog(ptProgram) << std::endl;
    }
    glValidateProgram(ptProgram);
    if (gl::checkForProgramErr(ptProgram, GL_VALIDATE_STATUS)) {
      std::cerr << gl::getProgramInfoLog(ptProgram) << std::endl;
    }


    //bgVertShader = glCreateShader(GL_VERTEX_SHADER);
    //loadFile(BG_VERTEX_SHADER_SRC_PATH, source);
    //src = source.c_str();
    //len = (int)source.length();
    //glShaderSource(bgVertShader, 1, &src, &len);

    //bgFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    //loadFile(BG_FRAGMENT_SHADER_SRC_PATH, source);
    //src = source.c_str();
    //len = (int)source.length();
    //glShaderSource(bgFragShader, 1, &src, &len);

    //bgProgram = glCreateProgram();
    //glCompileShader(bgVertShader);
    //if (gl::checkForShaderErr(bgVertShader, GL_COMPILE_STATUS)) {
    //  std::cerr << gl::getProgramInfoLog(bgVertShader) << std::endl;
    //}
    //glAttachShader(bgProgram, bgVertShader);

    //glCompileShader(bgFragShader);
    //if (gl::checkForShaderErr(bgFragShader, GL_COMPILE_STATUS)) {
    //  std::cerr << gl::getProgramInfoLog(bgFragShader) << std::endl;
    //}
    //glAttachShader(bgProgram, bgFragShader);

    //for (int i = 0; i < NUM_BG_SHADER_ATTR; i++) {
    //  glBindAttribLocation(bgProgram, i, glBgShaderIns[i]);
    //}
    //glLinkProgram(bgProgram);
    //if (gl::checkForProgramErr(bgProgram, GL_LINK_STATUS)) {
    //  std::cerr << gl::getProgramInfoLog(bgProgram) << std::endl;
    //}
    //glValidateProgram(bgProgram);
    //if (gl::checkForProgramErr(bgProgram, GL_VALIDATE_STATUS)) {
    //  std::cerr << gl::getProgramInfoLog(bgProgram) << std::endl;
    //}


    clContext = context;

    int clErr;
    std::string clSource;
    loadFile(RENDER_KERNEL_SRC_PATH, clSource);
    clProgram = cl::Program(clContext, clSource);
    clErr = clProgram.build();
    if (clErr != 0) {
      std::cerr << clProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
      throw clErr;
    }

    clQueue = cl::CommandQueue(clContext, device);

    isHardwAccInit = true;
  }
  void createParticleRing(uint rnParticles, float rr, vector rn,
      float rdr, float rdtheta, float rdphi,
      uint nColors, colorBlur* rPtColorPalette) {

    float* newPtR = new float[nParticles + rnParticles];
    float* newPtTheta = new float[nParticles + rnParticles];
    float* newPtPhi = new float[nParticles + rnParticles];
    color* newPtColor = new color[nParticles + rnParticles];
    for (ulong i = 0; i < nParticles; i++) {
      newPtR[i] = ptR[i];
      newPtTheta[i] = ptTheta[i];
      newPtPhi[i] = ptPhi[i];
      newPtColor[i] = ptColor[i];
    }
    for (ulong i = nParticles; i < nParticles + rnParticles; i++) {
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
      newPtColor[i] = { 1.0f, 0.3f, 0.0f, 0.05f };//selectObject<color>(nColors, reinterpret_cast<std::pair<color, float>*>(rPtColorPalette));
    }

    delete[] ptR;
    delete[] ptTheta;
    delete[] ptPhi;
    delete[] ptColor;
    ptR = newPtR;
    ptTheta = newPtTheta;
    ptPhi = newPtPhi;
    ptColor = newPtColor;
    nParticles += rnParticles;

    glGenVertexArrays(1, &glParticles);
    glBindVertexArray(glParticles);

    glGenBuffers(1, &glPtPositionBuf);
    glBindBuffer(GL_ARRAY_BUFFER, glPtPositionBuf);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(cl_float4), nullptr, GL_STATIC_DRAW);

    glFinish();

    cl::Kernel kernelGetPtPositions(clProgram, kernelNames[GET_PT_POSITIONS]);
    cl::Buffer posBuffer = cl::BufferGL(clContext, CL_MEM_READ_WRITE, glPtPositionBuf);
    cl::Buffer rBuffer = cl::Buffer(clContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), ptR);
    cl::Buffer thetaBuffer = cl::Buffer(clContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), ptTheta);
    cl::Buffer phiBuffer = cl::Buffer(clContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nParticles * sizeof(float), ptPhi);

    cl::setKernelArgs(kernelGetPtPositions, posBuffer, rBuffer, thetaBuffer, phiBuffer);
    std::vector<cl::Memory> glObjects = { posBuffer };
    clQueue.enqueueAcquireGLObjects(&glObjects);
    clQueue.finish();
    clQueue.enqueueNDRangeKernel(kernelGetPtPositions, cl::NullRange, cl::NDRange(nParticles));
    clQueue.finish();
    clQueue.enqueueReleaseGLObjects(&glObjects);
    clQueue.finish();

    cl::finish();
    glFlush();

    glEnableVertexAttribArray(PT_POS);
    glVertexAttribPointer(PT_POS, 4, GL_FLOAT, false, 0, (void*)0);

    glGenBuffers(1, &ptColorBuf);
    glBindBuffer(GL_ARRAY_BUFFER, ptColorBuf);
    glBufferData(GL_ARRAY_BUFFER, (int)nParticles * sizeof(color), ptColor, GL_STATIC_DRAW);
    glEnableVertexAttribArray(PT_COLOR);
    glVertexAttribPointer(PT_COLOR, 4, GL_FLOAT, false, 0, (void*)0);

    glBindVertexArray(0);
  }
  void clearParticleRings()
  {
    delete[] ptR;
    delete[] ptTheta;
    delete[] ptPhi;
    delete[] ptColor;

    ptR = new float[0];
    ptTheta = new float[0];
    ptPhi = new float[0];
    ptColor = new color[0];
    nParticles = 0;
  }
  void setObserver(camera _observer) {
    _observer.lookDir = glm::normalize(_observer.lookDir);
    _observer.upDir = glm::normalize(_observer.upDir);
    observer = _observer;

    glViewProj = glGetUniformLocation(ptProgram, glPtShaderUniforms[PT_VIEW_PROJ]);
    persp = glm::perspective(glm::radians(observer.fov), observer.aspect, observer.zNear, observer.zFar);
    glm::mat4 vp = persp * glm::lookAt(observer.pos, observer.pos + observer.lookDir, observer.upDir);
    glUniformMatrix4fv(glViewProj, 1, false, &vp[0][0]);

    t0 = std::chrono::high_resolution_clock::now();
  }
  void setBackground(uint sData, byte* data, uint width, uint height, uint bpp) {
    bgSImageData = sData;
    bgWidth = width;
    bgHeight = height;
    bgBpp = bpp;
    bgImageData = data;

    //const glm::vec3 bgPos[] = {
    //  { -1.0f, -1.0f , 0.0f },
    //  {  1.0f, -1.0f , 0.0f },
    //  { -1.0f,  1.0f , 0.0f },
    //  {  1.0f,  1.0f , 0.0f }
    //};

    //glGenVertexArrays(1, &glBg);
    //glBindVertexArray(glBg);

    //glGenBuffers(1, &glBgPositionBuffer);
    //glBindBuffer(GL_ARRAY_BUFFER, glBgPositionBuffer);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(bgPos), bgPos, GL_STATIC_DRAW);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);

    //glGenBuffers(1, &glBgTexCoordBuffer);
    //glBindBuffer(GL_ARRAY_BUFFER, glBgTexCoordBuffer);
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, (void*)0);

    //glGenTextures(1, &glBgTexture);
    //glBindTexture(GL_TEXTURE_2D, glBgTexture);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bgWidth, bgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bgImageData);
  }
  void render()
  {
    if (!isInit)
      throw "Rendering is not initialized.";
    if (hardwAcc && !isHardwAccInit)
      throw "Hardware acceleration for rendering is not initialized.";

    _isRendering = true;
    if (isRel) {
      if (hardwAcc)
        renderRelGpu();
      else renderRel();
    }
    else {
			if (hardwAcc) {
				renderClaGpu();
			}
      else renderCla();
    }
    _isRendering = false;
  }
  void renderCla() {
    
  }
  void renderClaGpu() {
    glClear(GL_COLOR_BUFFER_BIT);

    //float fovY2 = 0.5f * glm::radians(observer.fov);
    //float fovX2 = atan(observer.aspect * tan(fovY2));
    //float phi1 = atan2(observer.lookDir.y, observer.lookDir.x) + fovX2;
    //float phi2 = phi1 - 2.0f * fovX2;
    //float theta1 = acos(observer.lookDir.z / observer.lookDir.length()) - fovY2;
    //float theta2 = theta1 + 2.0f * fovY2;
    //bgTexCoord[0] = { phi1, theta1 };
    //bgTexCoord[1] = { phi1, theta1 };
    //bgTexCoord[2] = { phi2, theta2 };
    //bgTexCoord[3] = { phi1, theta2 };

    //glUseProgram(bgProgram);
    //glBindVertexArray(glBg);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(bgTexCoord), bgTexCoord, GL_STATIC_DRAW);
    //glActiveTexture(GL_TEXTURE0 + 0);
    //glBindTexture(GL_TEXTURE_2D, glBgTexture);
    //glDrawArrays(GL_QUADS, 0, 4);
    //glBindVertexArray(0);

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    float dt = (std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() % 10000) / 1000.0f;
    glm::mat4 rot = glm::rotate(2.0f * (float)M_PI * dt / 10.0f, observer.upDir);
    glm::vec3 newPos = rot * glm::vec4(observer.pos, 1.0);
    glm::vec3 newLookDir = rot * glm::vec4(observer.lookDir, 1.0);
    glm::mat4 vp = persp * glm::lookAt(newPos, newPos + newLookDir, observer.upDir);

    glUseProgram(ptProgram);
    glBindVertexArray(glParticles);
    glUniformMatrix4fv(glViewProj, 1, false, &vp[0][0]);
    glDrawArrays(GL_POINTS, 0, (int)nParticles);
    glBindVertexArray(0);
  }
  void renderRel() {

  }
  void renderRelGpu() {

  }
  void close() {
    glDeleteBuffers(1, &glPtPositionBuf);
    glDeleteVertexArrays(1, &glParticles);

    glDetachShader(ptProgram, ptVertShader);
    glDeleteShader(ptVertShader);
    glDetachShader(ptProgram, ptFragShader);
    glDeleteShader(ptFragShader);
    glDeleteProgram(ptProgram);

    delete[] ptR;
    delete[] ptTheta;
    delete[] ptPhi;
    delete[] ptColor;

    delete[] bgImageData;
  }
}
