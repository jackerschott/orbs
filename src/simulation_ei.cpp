#define _USE_MATH_DEFINES
#define PI float(M_PI)
#define PI_2 float(M_PI_2)

#include <chrono>
#include <climits>
#include <cmath>
#include <iostream>
#include <string>
#include <time.h>
#include <utility>

#include "clWrapper/clwrap.hpp"
#include "glWrapper/glwrap.hpp"
#include "simulation_ei.hpp"

namespace sl {
  // Physics data
  float rs;
  camera observer;
  bool isRel = false;

  // GPU Rendering programs
  GLuint bgRenderProg;
  GLuint bgVertShader;
  GLuint bgFragShader;

  GLuint ptRenderProg;
  GLuint ptVertShader;
  GLuint ptFragShader;

  // GPU buffers
  GLuint glBackground;
  GLuint glBgTexture;
  GLuint glBgPosBuf;
  GLuint glBgTexCoordBuf;

  std::vector<uint> nClusterPts;
  std::vector<GLuint> glClusterPts;
  std::vector<GLuint> glPtPosBufs;
  std::vector<GLuint> glPtColorBufs;
  GLuint glSelPts;
  int sCluster = -1;

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
  cl::Program clPtGenProgram;
  cl::Program clRngProgram;
  cl::Context clContext;
  cl::CommandQueue clQueue;

  cl::Kernel kerGetEllipticPtDistr;
  cl::Kernel kerGetPtColors;

  cl::Kernel kerGenSamples;
  cl::Kernel kerGenFloatSamples;
  cl::Kernel kerGenGaussianSamples;

  // GPU computation buffers
  cl::Buffer clPosBuf;
  cl::Buffer clPaletteBuf;
  cl::Buffer clBlurSizesBuf;
  cl::Buffer clColorBuf;
  cl_float4* posBuf;
  cl_float4* colorBuf;

  // Organisation Details
  slConfig config = CONFIG_CLOSED;

  uint64 getRngOff();

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
#if _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl::msgCallback, 0);
#endif
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build rendering programs
    std::string errLog;
    if (!gl::createProgram(PT_VERTEX_SHADER_SRC_PATH, PT_FRAGMENT_SHADER_SRC_PATH,
      glPtShaderInNames, &ptRenderProg, &ptVertShader, &ptFragShader, &errLog)) {
      std::cerr << errLog << std::endl;
    }
    if (!gl::createProgram(BG_VERTEX_SHADER_SRC_PATH, BG_FRAGMENT_SHADER_SRC_PATH,
      glBgShaderInNames, &bgRenderProg, &bgVertShader, &bgFragShader, &errLog)) {
      std::cerr << errLog << std::endl;
    }

    glUseProgram(ptRenderProg);
    viewProjn = glGetUniformLocation(ptRenderProg, glPtShaderUniNames[PT_VIEW_PROJ]);
    glUseProgram(0);

    // Initialize Background rendering Buffers
    glGenVertexArrays(1, &glBackground);
    glBindVertexArray(glBackground);

    glGenBuffers(1, &glBgPosBuf);
    glBindBuffer(GL_ARRAY_BUFFER, glBgPosBuf);
    glEnableVertexAttribArray(BG_POS);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);

    glGenBuffers(1, &glBgTexCoordBuf);
    glBindBuffer(GL_ARRAY_BUFFER, glBgTexCoordBuf);
    glEnableVertexAttribArray(BG_TEX_COORD);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, (void*)0);

    glBindVertexArray(0);

    // Initialize background texture
    glGenTextures(1, &glBgTexture);
    glBindTexture(GL_TEXTURE_2D, glBgTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Initialize GPU computation programs
    int clErr;
    std::string clSource;
    clContext = context;

    loadFile(PTGEN_KERNEL_SRC_PATH, &clSource);
    clPtGenProgram = cl::Program(clContext, clSource);
    clErr = clPtGenProgram.build("-I" KERNEL_PATH);
    if (clErr != 0) {
      std::cerr << "Program build error " << clErr << ": " << clPtGenProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
      throw clErr;
    }
    loadFile(RNG_KERNEL_SRC_PATH, &clSource);
    clRngProgram = cl::Program(clContext, clSource);
    clErr = clRngProgram.build("-I" KERNEL_PATH);
    if (clErr != 0) {
      std::cerr << "Program build error " << clErr << ": " << clRngProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
      throw clErr;
    }

    clQueue = cl::CommandQueue(clContext, device);

    kerGetEllipticPtDistr = cl::Kernel(clPtGenProgram, ptgenKernelNames[KERNEL_GET_ELLIPTIC_PT_DISTR]);
    kerGetPtColors = cl::Kernel(clPtGenProgram, ptgenKernelNames[KERNEL_GET_PT_COLORS]);

    kerGenSamples = cl::Kernel(clRngProgram, rngKernelNames[KERNEL_GEN_SAMPLES]);
    kerGenFloatSamples = cl::Kernel(clRngProgram, rngKernelNames[KERNEL_GEN_FLOAT_SAMPLES]);
    kerGenGaussianSamples = cl::Kernel(clRngProgram, rngKernelNames[KERNEL_GEN_GAUSSIAN_SAMPLES]);

    config = CONFIG_INIT;
  }

  void createEllipticCluster(uint nParticles, float a, float b, vector n, float dr, float dz,
    uint nColors, color* palette, float* blurSizes) {
    std::chrono::time_point t1 = std::chrono::high_resolution_clock::now();
    
    int err = 0;
    posBuf = new cl_float4[nParticles];
    colorBuf = new cl_float4[nParticles];
    clPosBuf = cl::Buffer(clContext, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_USE_HOST_PTR, nParticles * sizeof(cl_float4), posBuf, &err);
    clPaletteBuf = cl::Buffer(clContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nColors * sizeof(cl_float4), palette, &err);
    clBlurSizesBuf = cl::Buffer(clContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nColors * sizeof(float), blurSizes, &err);
    clColorBuf = cl::Buffer(clContext, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_USE_HOST_PTR, nParticles * sizeof(cl_float4), colorBuf, &err);

    cl::Buffer uSamplesBuf1 = cl::Buffer(clContext, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, nParticles * sizeof(float), &err);
    cl::Buffer uSamplesBuf2 = cl::Buffer(clContext, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, nParticles * sizeof(float), &err);
    cl::Buffer gSamplesBuf1 = cl::Buffer(clContext, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, nParticles * sizeof(float), &err);
    cl::Buffer gSamplesBuf2 = cl::Buffer(clContext, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, nParticles * sizeof(float), &err);

    err = kerGenFloatSamples.setArg(0, nParticles);
    err = kerGenFloatSamples.setArg(1, getRngOff());
    err = kerGenFloatSamples.setArg(2, uSamplesBuf1);
    err = clQueue.enqueueNDRangeKernel(kerGenFloatSamples, cl::NullRange, cl::NDRange(4096));
    err = kerGenFloatSamples.setArg(1, getRngOff());
    err = kerGenFloatSamples.setArg(2, uSamplesBuf2);
    err = clQueue.enqueueNDRangeKernel(kerGenFloatSamples, cl::NullRange, cl::NDRange(4096));

    err = kerGenGaussianSamples.setArg(0, nParticles);
    err = kerGenGaussianSamples.setArg(1, getRngOff());
    err = kerGenGaussianSamples.setArg(2, gSamplesBuf1);
    err = clQueue.enqueueNDRangeKernel(kerGenGaussianSamples, cl::NullRange, cl::NDRange(4096));
    err = kerGenGaussianSamples.setArg(1, getRngOff());
    err = kerGenGaussianSamples.setArg(2, gSamplesBuf2);
    err = clQueue.enqueueNDRangeKernel(kerGenGaussianSamples, cl::NullRange, cl::NDRange(4096));
    clQueue.finish();

    float eps = sqrt(1 - (b * b) / (a * a));
    glm::mat4 rot = glm::rotate(n.z / glm::length(n), glm::vec3(-n.y, n.x, 0.0));
    glm::mat4 rotArg = glm::transpose(rot);

    err = kerGetEllipticPtDistr.setArg(0, nParticles);
    err = kerGetEllipticPtDistr.setArg(1, b);
    err = kerGetEllipticPtDistr.setArg(2, eps);
    err = kerGetEllipticPtDistr.setArg(3, *reinterpret_cast<cl_float16*>(&rotArg));
    err = kerGetEllipticPtDistr.setArg(4, dr);
    err = kerGetEllipticPtDistr.setArg(5, dz);
    err = kerGetEllipticPtDistr.setArg(6, uSamplesBuf1);
    err = kerGetEllipticPtDistr.setArg(7, gSamplesBuf1);
    err = kerGetEllipticPtDistr.setArg(8, gSamplesBuf2);
    err = kerGetEllipticPtDistr.setArg(9, clPosBuf);
    err = clQueue.enqueueNDRangeKernel(kerGetEllipticPtDistr, cl::NullRange, cl::NDRange(nParticles));
    clQueue.finish();
    err = clQueue.enqueueReadBuffer(clPosBuf, false, 0, nParticles * sizeof(cl_float4), posBuf);

    err = kerGetPtColors.setArg(0, nColors);
    err = kerGetPtColors.setArg(1, clPaletteBuf);
    err = kerGetPtColors.setArg(2, clBlurSizesBuf);
    err = kerGetPtColors.setArg(3, uSamplesBuf2);
    err = kerGetPtColors.setArg(4, clColorBuf);
    err = clQueue.enqueueNDRangeKernel(kerGetPtColors, cl::NullRange, cl::NDRange(nParticles));
    err = clQueue.finish();
    err = clQueue.enqueueReadBuffer(clColorBuf, false, 0, nParticles * sizeof(cl_float4), colorBuf);
    clQueue.finish();

    // Initialize Particle Buffers
    GLuint glEllipsePts;
    GLuint glPtPosBuf;
    GLuint glPtColorBuf;
    glGenVertexArrays(1, &glEllipsePts);
    glBindVertexArray(glEllipsePts);

    glGenBuffers(1, &glPtPosBuf);
    glBindBuffer(GL_ARRAY_BUFFER, glPtPosBuf);
    glEnableVertexAttribArray(PT_POS);
    glVertexAttribPointer(PT_POS, 4, GL_FLOAT, false, 0, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(cl_float4), posBuf, GL_STATIC_DRAW);

    glGenBuffers(1, &glPtColorBuf);
    glBindBuffer(GL_ARRAY_BUFFER, glPtColorBuf);
    glEnableVertexAttribArray(PT_COLOR);
    glVertexAttribPointer(PT_COLOR, 4, GL_FLOAT, false, 0, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(cl_float4), colorBuf, GL_STATIC_DRAW);

    nClusterPts.push_back(nParticles);
    glClusterPts.push_back(glEllipsePts);
    glPtPosBufs.push_back(glPtPosBuf);
    glPtColorBufs.push_back(glPtColorBuf);

    delete[] posBuf;
    delete[] colorBuf;

    std::chrono::time_point t2 = std::chrono::high_resolution_clock::now();
    float calcTime = 0.001f * std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "Cluster calculation time: " << calcTime << " s" << std::endl;
  }
  void clearClusters()
  {
    // Clear particle GPU buffers
    for (int i = 0; i < glClusterPts.size(); i++) {
      glDeleteBuffers(1, &glPtPosBufs[i]);
      glDeleteBuffers(1, &glPtColorBufs[i]);
      glDeleteVertexArrays(1, &glClusterPts[i]);
    }
    nClusterPts.clear();
    glClusterPts.clear();
    glPtPosBufs.clear();
    glPtColorBufs.clear();
  }
  void selectCluster(int index) {
    sCluster = index;
  }
  void deselectClusters() {
    sCluster = -1;
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

    config = (slConfig)(config | CONFIG_HAS_CAMERA);
  }
  void setBackgroundTex(uint sData, byte* data, uint width, uint height, uint bpp) {
    assert((config & CONFIG_INIT) != 0 && (config & CONFIG_HAS_BG_TEX) == 0);

    // for (int i = 0; i <= sData; i++) {
    //   if (data[i] != '\0' && data[i] != '\255') {
    //     std::cout << (int)data[i] << std::endl;
    //   }
    // }

    // Set background shader inputs
    glBindVertexArray(glBackground);

    glBindBuffer(GL_ARRAY_BUFFER, glBgPosBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgPos), bgPos, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, glBgTexCoordBuf);

    glBindVertexArray(0);

    // Set background texture to passed image
    glBindTexture(GL_TEXTURE_2D, glBgTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    config = (slConfig)(config | CONFIG_HAS_BG_TEX);
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

  void updateParticlesClassic(float time) {

  }
  void updateParticlesRelatvistic(float time) {

  }
  void renderClassic() {
    assert(config == CONFIG_INIT_FOR_RENDER);
    config = (slConfig)(config | CONFIG_RENDERING);

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
    glBindBuffer(GL_ARRAY_BUFFER, glBgTexCoordBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgTexCoord), bgTexCoord, GL_STATIC_DRAW);
    // Bind Background Texture
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, glBgTexture);
    // Render Background
    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);

    // Render Particles
    glUseProgram(ptRenderProg);
    for (int i = 0; i < glClusterPts.size(); i++) {
      glBindVertexArray(glClusterPts[i]);
      glDrawArrays(GL_POINTS, 0, (int)(nClusterPts[i]));
      glBindVertexArray(0);
    }
    glUseProgram(0);

    config = (slConfig)(config & ~CONFIG_RENDERING);
  }
  void renderRelativistic() {
    assert(config == CONFIG_INIT_FOR_RENDER);
    config = (slConfig)(config | CONFIG_RENDERING);



    config = (slConfig)(config & ~CONFIG_RENDERING);
  }

  void close() {
    assert((config & CONFIG_INIT) != 0);

    clearClusters();

    glDeleteTextures(1, &glBgTexture);

    glDeleteBuffers(1, &glBgPosBuf);
    glDeleteBuffers(1, &glBgTexCoordBuf);
    glDeleteVertexArrays(1, &glBackground);

    glDetachShader(ptRenderProg, ptVertShader);
    glDeleteShader(ptVertShader);
    glDetachShader(ptRenderProg, ptFragShader);
    glDeleteShader(ptFragShader);
    glDeleteProgram(ptRenderProg);

    config = CONFIG_CLOSED;
  }

  uint64 getRngOff() {
    uint64 dt = time(0);
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    t1 = std::chrono::floor<std::chrono::seconds>(t1);
    return dt * 1000000000ULL + (uint64)std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
  }
}
