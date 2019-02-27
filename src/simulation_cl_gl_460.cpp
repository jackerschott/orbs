#define _USE_MATH_DEFINES
#define PI float(M_PI)
#define PI_2 float(M_PI_2)

#include <chrono>
#include <climits>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <time.h>
#include <utility>

#include "clWrapper/clwrap.hpp"
#include "glWrapper/glwrap.hpp"
#include "simulation_cl_gl_460.hpp"

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
    { -1.0f, -1.0f,  0.0f },
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

  // Organisation Details
  slConfig config = CONFIG_CLOSED;

  uint64 getRngOff();
  void buildClusterProg();
  void buildBgProg();
  bool getShaderSrc(const char* path, char* src);
  void GLAPIENTRY msgCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam);
  const char* getErrSource(GLenum source);
  const char* getErrType(GLenum type);
  const char* getErrSeverity(GLenum severity);
  std::string getErrMsg(int err);

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
    buildClusterProg();
    buildBgProg();

    glUseProgram(ptRenderProg);
    viewProjn = glGetUniformLocation(ptRenderProg, "viewProj");
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
    clContext = cl::Context(cl::Device::getDefault());//context;

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
    // Initialize Particle Buffers
    GLuint glEllipsePts;
    GLuint glPtPosBuf;
    GLuint glPtColorBuf;

    std::string glErr;
    glGenVertexArrays(1, &glEllipsePts);
    glBindVertexArray(glEllipsePts);

    glGenBuffers(1, &glPtPosBuf);
    glBindBuffer(GL_ARRAY_BUFFER, glPtPosBuf);
    glEnableVertexAttribArray(PT_POS);
    glVertexAttribPointer(PT_POS, 4, GL_FLOAT, false, 0, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(cl_float4), nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &glPtColorBuf);
    glBindBuffer(GL_ARRAY_BUFFER, glPtColorBuf);
    glEnableVertexAttribArray(PT_COLOR);
    glVertexAttribPointer(PT_COLOR, 4, GL_FLOAT, false, 0, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(cl_float4), nullptr, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glFinish();

    int err = 0;
    clPosBuf = cl::BufferGL(clContext, CL_MEM_READ_WRITE, glPtPosBuf, &err);
    clPaletteBuf = cl::Buffer(clContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nColors * sizeof(cl_float4), palette, &err);
    clBlurSizesBuf = cl::Buffer(clContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, nColors * sizeof(float), blurSizes, &err);
    clColorBuf = cl::BufferGL(clContext, CL_MEM_READ_WRITE, glPtColorBuf, &err);

    std::vector<cl::Memory> glMem({ clPosBuf, clColorBuf });
    clQueue.enqueueAcquireGLObjects(&glMem);

    cl::Buffer uSamplesBuf1 = cl::Buffer(clContext, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, nParticles * sizeof(uint), &err);
    cl::Buffer uSamplesBuf2 = cl::Buffer(clContext, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, nParticles * sizeof(uint), &err);
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
    glm::mat4 cl_rot = glm::transpose(rot);

    glBindVertexArray(glEllipsePts);
    glBindBuffer(GL_ARRAY_BUFFER, glPtPosBuf);
    glFinish();
    err = kerGetEllipticPtDistr.setArg(0, nParticles);
    err = kerGetEllipticPtDistr.setArg(1, b);
    err = kerGetEllipticPtDistr.setArg(2, eps);
    err = kerGetEllipticPtDistr.setArg(3, *reinterpret_cast<cl_float16*>(&cl_rot));
    err = kerGetEllipticPtDistr.setArg(4, dr);
    err = kerGetEllipticPtDistr.setArg(5, dz);
    err = kerGetEllipticPtDistr.setArg(6, uSamplesBuf1);
    err = kerGetEllipticPtDistr.setArg(7, gSamplesBuf1);
    err = kerGetEllipticPtDistr.setArg(8, gSamplesBuf2);
    err = kerGetEllipticPtDistr.setArg(9, clPosBuf);
    std::vector<cl::Memory> posMem({ clPosBuf });
    err = clQueue.enqueueNDRangeKernel(kerGetEllipticPtDistr, cl::NullRange, cl::NDRange(nParticles));
    clQueue.finish();

    glBindBuffer(GL_ARRAY_BUFFER, glPtColorBuf);
    err = kerGetPtColors.setArg(0, nColors);
    err = kerGetPtColors.setArg(1, clPaletteBuf);
    err = kerGetPtColors.setArg(2, clBlurSizesBuf);
    err = kerGetPtColors.setArg(3, uSamplesBuf2);
    err = kerGetPtColors.setArg(4, clColorBuf);
    err = clQueue.enqueueNDRangeKernel(kerGetPtColors, cl::NullRange, cl::NDRange(nParticles));
    err = clQueue.finish();

    clQueue.enqueueReleaseGLObjects(&glMem);
    glBindVertexArray(0);

    nClusterPts.push_back(nParticles);
    glClusterPts.push_back(glEllipsePts);
    glPtPosBufs.push_back(glPtPosBuf);
    glPtColorBufs.push_back(glPtColorBuf);
  }
  void clearClusters() {
    // Clear particle GPU buffers
    for (uint i = 0; i < glClusterPts.size(); i++) {
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

    bgTexCoord[0] = glm::vec2(1.0 - phi2 / (2.0f * M_PI), theta1 / M_PI);
    bgTexCoord[1] = glm::vec2(1.0 - phi1 / (2.0f * M_PI), theta1 / M_PI);
    bgTexCoord[2] = glm::vec2(1.0 - phi1 / (2.0f * M_PI), theta2 / M_PI);
    bgTexCoord[3] = glm::vec2(1.0 - phi2 / (2.0f * M_PI), theta1 / M_PI);
    bgTexCoord[4] = glm::vec2(1.0 - phi1 / (2.0f * M_PI), theta2 / M_PI);
    bgTexCoord[5] = glm::vec2(1.0 - phi2 / (2.0f * M_PI), theta2 / M_PI);

    glUseProgram(bgRenderProg);
    glBindVertexArray(glBackground);
    // Set Background Texture Coordinates
    glBindBuffer(GL_ARRAY_BUFFER, glBgTexCoordBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgTexCoord), bgTexCoord, GL_STATIC_DRAW);
    // Bind Background Texture
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, glBgTexture);
    // Render Background
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);

    // Render Particles
    glUseProgram(ptRenderProg);
    for (uint i = 0; i < glClusterPts.size(); i++) {
      glBindVertexArray(glClusterPts[i]);
      glDrawArrays(GL_POINTS, 0, (int)nClusterPts[i]);
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
    uint64 dt = (uint64)time(0);
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    t1 = std::chrono::floor<std::chrono::seconds>(t1);
    return dt * 1000000000ULL + (uint64)std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
  }

    void buildClusterProg() {
    char* src;
    int len;
    int success;
    char log[0x400];

    // Create shaders
    cluster_vs = glCreateShader(GL_VERTEX_SHADER);
    if (!getShaderSrc(SHADER_PATH "cluster.vert", &src, &len)) {
      std::cerr << SHADER_PATH "cluster.vert" << " could not be opened." << std::endl;
      throw;
    }
    glShaderSource(cluster_vs, 1, &src, &len);
    delete[] src;

    cluster_fs = glCreateShader(GL_FRAGMENT_SHADER);
    if (!getShaderSrc(SHADER_PATH "cluster.frag", &src, &len)) {
      std::cerr << SHADER_PATH "cluster.frag" << "could not be opened." << std::endl;
      throw;
    }
    glShaderSource(cluster_fs, 1, &src, &len);
    delete[] src;

    // Compile shaders
    cluster_prog = glCreateProgram();
    glCompileShader(cluster_vs);
    glGetShaderiv(cluster_vs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(cluster_vs, sizeof(log), NULL, log);
      std::cerr << "Cluster vertex shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(cluster_prog, cluster_vs);

    glCompileShader(cluster_fs);
    glGetShaderiv(cluster_fs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(cluster_fs, sizeof(log), NULL, log);
      std::cerr << "Cluster fragment shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(cluster_prog, cluster_fs);

    // Bind attribute locations
    glBindAttribLocation(cluster_prog, 0, "pos");
    glBindAttribLocation(cluster_prog, 1, "color");

    // Link Program
    glLinkProgram(cluster_prog);
    glGetProgramiv(cluster_prog, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(cluster_prog, sizeof(log), NULL, log);
      std::cerr << "Cluster program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Validate Program
    glValidateProgram(cluster_prog);
    glGetProgramiv(cluster_prog, GL_VALIDATE_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(cluster_prog, sizeof(log), NULL, log);
      std::cerr << "Cluster program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
  }
  void buildBgProg() {
    char* src;
    int len;
    int success;
    char log[0x400];

    // Create shaders
    bg_vs = glCreateShader(GL_VERTEX_SHADER);
    if (!getShaderSrc(SHADER_PATH "bg.vert", &src, &len)) {
      std::cerr << SHADER_PATH "bg.vert" << " could not be opened." << std::endl;
      throw;
    }
    glShaderSource(bg_vs, 1, &src, &len);
    delete[] src;

    bg_fs = glCreateShader(GL_FRAGMENT_SHADER);
    if (!getShaderSrc(SHADER_PATH "bg.frag", &src, &len)) {
      std::cerr << SHADER_PATH "bg.frag" << "could not be opened." << std::endl;
      throw;
    }
    glShaderSource(bg_fs, 1, &src, &len);
    delete[] src;

    // Compile shaders
    bg_prog = glCreateProgram();
    glCompileShader(bg_vs);
    glGetShaderiv(bg_vs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(bg_vs, sizeof(log), NULL, log);
      std::cerr << "Bg vertex shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(bg_prog, bg_vs);

    glCompileShader(bg_fs);
    glGetShaderiv(bg_fs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(bg_fs, sizeof(log), NULL, log);
      std::cerr << "Bg fragment shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(bg_prog, bg_fs);

    // Bind attribute locations
    glBindAttribLocation(bg_prog, 0, "pos");
    glBindAttribLocation(bg_prog, 1, "texCoord");

    // Link Program
    glLinkProgram(bg_prog);
    glGetProgramiv(bg_prog, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(bg_prog, sizeof(log), NULL, log);
      std::cerr << "Bg program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Validate Program
    glValidateProgram(bg_prog);
    glGetProgramiv(bg_prog, GL_VALIDATE_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(bg_prog, sizeof(log), NULL, log);
      std::cerr << "Bg program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
  }
  bool getShaderSrc(const char* path, char** src, int* srcLen) {
    FILE* fp = fopen(path, "r");
    if (fp == NULL)
      return false;
    
    fseek(fp, 0, SEEK_END);
    *srcLen = (int)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    *src = new char[*srcLen];
    *src[0] = '\0';
    char* line = NULL;
    unsigned long len = 0;
    long read;
    while ((read = getline(&line, &len, fp)) != -1) {
      strcat(*src, line);
    }

    fclose(fp);
    if (line)
      free(line);
    return true;
  }
  void GLAPIENTRY msgCallback(GLenum source, GLenum type, UNUSED GLuint id, GLenum severity,
    UNUSED GLsizei length, const GLchar* message, UNUSED const void* userParam) {
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
      fprintf(stderr, "%s\t %s, type: %s, source: %s\n",
        getErrSeverity(severity), message, getErrType(type), getErrSource(source));
    }
  }
  const char* getErrSource(GLenum source) {
    switch (source)
    {
    case GL_DEBUG_SOURCE_API: return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window system";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third party";
    case GL_DEBUG_SOURCE_APPLICATION: return "Application";
    case GL_DEBUG_SOURCE_OTHER: return "Other";
    default: return "Unknown error source";
    }
  }
  const char* getErrType(GLenum type) {
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR: return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated behavior";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined behavior";
    case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
    case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
    case GL_DEBUG_TYPE_MARKER: return "Marker";
    case GL_DEBUG_TYPE_PUSH_GROUP: return "Push group";
    case GL_DEBUG_TYPE_POP_GROUP: return "Pop Group";
    case GL_DEBUG_TYPE_OTHER: return "Other";
    default: return "Unknown error type";
    }
  }
  const char* getErrSeverity(GLenum severity) {
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH: return "Error";
    case GL_DEBUG_SEVERITY_MEDIUM: return "Major warning";
    case GL_DEBUG_SEVERITY_LOW: return "Warning";
    case GL_DEBUG_SEVERITY_NOTIFICATION: return "Note";
    default: return "Unknown error severity";
    }
  }

  std::string getErrMsg(int err) {
		switch (err) {
		case 0: return "CL_SUCCESS";
		case -1: return "CL_DEVICE_NOT_FOUND";
		case -2: return "CL_DEVICE_NOT_AVAILABLE";
		case -3: return "CL_COMPILER_NOT_AVAILABLE";
		case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case -5: return "CL_OUT_OF_RESOURCES";
		case -6: return "CL_OUT_OF_HOST_MEMORY";
		case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case -8: return "CL_MEM_COPY_OVERLAP";
		case -9: return "CL_IMAGE_FORMAT_MISMATCH";
		case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case -11: return "CL_BUILD_PROGRAM_FAILURE";
		case -12: return "CL_MAP_FAILURE";
		case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		case -15: return "CL_COMPILE_PROGRAM_FAILURE";
		case -16: return "CL_LINKER_NOT_AVAILABLE";
		case -17: return "CL_LINK_PROGRAM_FAILURE";
		case -18: return "CL_DEVICE_PARTITION_FAILED";
		case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

		case -30: return "CL_INVALID_VALUE";
		case -31: return "CL_INVALID_DEVICE_TYPE";
		case -32: return "CL_INVALID_PLATFORM";
		case -33: return "CL_INVALID_DEVICE";
		case -34: return "CL_INVALID_CONTEXT";
		case -35: return "CL_INVALID_QUEUE_PROPERTIES";
		case -36: return "CL_INVALID_COMMAND_QUEUE";
		case -37: return "CL_INVALID_HOST_PTR";
		case -38: return "CL_INVALID_MEM_OBJECT";
		case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case -40: return "CL_INVALID_IMAGE_SIZE";
		case -41: return "CL_INVALID_SAMPLER";
		case -42: return "CL_INVALID_BINARY";
		case -43: return "CL_INVALID_BUILD_OPTIONS";
		case -44: return "CL_INVALID_PROGRAM";
		case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
		case -46: return "CL_INVALID_KERNEL_NAME";
		case -47: return "CL_INVALID_KERNEL_DEFINITION";
		case -48: return "CL_INVALID_KERNEL";
		case -49: return "CL_INVALID_ARG_INDEX";
		case -50: return "CL_INVALID_ARG_VALUE";
		case -51: return "CL_INVALID_ARG_SIZE";
		case -52: return "CL_INVALID_KERNEL_ARGS";
		case -53: return "CL_INVALID_WORK_DIMENSION";
		case -54: return "CL_INVALID_WORK_GROUP_SIZE";
		case -55: return "CL_INVALID_WORK_ITEM_SIZE";
		case -56: return "CL_INVALID_GLOBAL_OFFSET";
		case -57: return "CL_INVALID_EVENT_WAIT_LIST";
		case -58: return "CL_INVALID_EVENT";
		case -59: return "CL_INVALID_OPERATION";
		case -60: return "CL_INVALID_GL_OBJECT";
		case -61: return "CL_INVALID_BUFFER_SIZE";
		case -62: return "CL_INVALID_MIP_LEVEL";
		case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case -64: return "CL_INVALID_PROPERTY";
		case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
		case -66: return "CL_INVALID_COMPILER_OPTIONS";
		case -67: return "CL_INVALID_LINKER_OPTIONS";
		case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

		case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
		case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
		case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
		case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
		case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
		case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
		default: return "Unknown OpenCL error";
		}
	}
}
