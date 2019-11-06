#include <chrono>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <time.h>
#include <utility>

#include "gl.hpp"
#include "simulation.hpp"

#define PI      3.14159265358979323846
#define PI2     6.28318530717958647693
#define PI_2    1.57079632679489661923

// This improvisational install path is only working if the binary is executed from workspace/project folder!
#define INSTALL_PATH "./"
#define SHADER_PATH INSTALL_PATH "src/shader/"

namespace sl {
  // GPU Programs
  gl::program* prog_rngUniform;
  gl::program* prog_rngGauss;
  gl::program* prog_clusterpos;
  gl::program* prog_clustercol;
  gl::program* prog_clusterdyn;
  gl::program* prog_renderBg;
  gl::program* prog_renderCluster;

  // Cluster data
  int sCluster = -1;

  std::vector<uint> nClusterVerts;
  std::vector<GLuint> glClusterVerts;
  std::vector<GLuint> glClusterPosBufs;
  std::vector<GLuint> glClusterColBufs;
  std::vector<GLuint> glClusterVelBufs;
  std::vector<GLuint> glClusterLBufs;
  std::vector<GLuint> glClusterEBufs;
  GLuint glSelPts;

  // Camera data
  camera cam;
  glm::mat4 P;
  glm::mat2 P_I;
  glm::mat4 V;

  // Background data
  uint bgTexWidth;
  uint bgTexHeight;
  std::vector<char>* bgTexData;

  GLuint glBgVerts;
  GLuint glBgPosBuf;
  GLuint glBgElementBuf;
  GLuint glBgTex;
  GLuint glBgRenderTex;

  // Organisation Details
  slConfig config;

  // Physical data
  float globalTime;

  void buildTestProg();

  uint64 getRngOff();
  void buildClusterProg();
  void buildBgProg();
  void buildUniformRngCompProg();
  void buildGaussRngCompProg();
  void buildClusterPosCompProg();
  void buildClusterColCompProg();
  bool readShaderSrc(const char* path, char** src, int* srcLen);
  void writeShaderSrc(const char* path, char* src, int srcLen);
  void GLAPIENTRY msgCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam);
  const char* getErrSource(GLenum source);
  const char* getErrType(GLenum type);
  const char* getErrSeverity(GLenum severity);

  bool isClosed() {
    return (config & CONFIG_CLOSED) != 0;
  }
  bool isInit() {
    return (config & CONFIG_INIT) != 0;
  }
  bool hasCamera() {
    return (config & CONFIG_HAS_CAM) != 0;
  }
  bool hasBgTex() {
    return true; // (config & CONFIG_HAS_BG_TEX) != 0;
  }

  void init() {
    assert(config == CONFIG_CLOSED);

    // Initialize OpenGL
    glewInit();
    // #if _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(msgCallback, 0);
    // #endif
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create shaders from files
    // Cluster generation shaders
    gl::shader cs_rngUniform(GL_COMPUTE_SHADER, SHADER_PATH "rng/rng_uniform.comp");
    cs_rngUniform.setIncludeSrc("uint64.glsl", SHADER_PATH "lib/uint64.glsl");
    cs_rngUniform.setIncludeSrc("rng_mwc_skip.glsl", SHADER_PATH "rng/rng_mwc_skip.glsl");
    cs_rngUniform.setIncludeSrc("rng_mwc64x.glsl", SHADER_PATH "rng/rng_mwc64x.glsl");
    gl::shader cs_rngGauss(GL_COMPUTE_SHADER, SHADER_PATH "rng/rng_gauss.comp");
    cs_rngGauss.setIncludeSrc("uint64.glsl", SHADER_PATH "lib/uint64.glsl");
    cs_rngGauss.setIncludeSrc("rng_mwc_skip.glsl", SHADER_PATH "rng/rng_mwc_skip.glsl");
    cs_rngGauss.setIncludeSrc("rng_mwc64x.glsl", SHADER_PATH "rng/rng_mwc64x.glsl");
    cs_rngGauss.setIncludeSrc("rng_tables.glsl", SHADER_PATH "rng/rng_tables.glsl");
    gl::shader cs_clusterpos(GL_COMPUTE_SHADER, SHADER_PATH "cluster_gen/clusterpos.comp");
    gl::shader cs_clustercol(GL_COMPUTE_SHADER, SHADER_PATH "cluster_gen/clustercol.comp");

    // Cluster dynamik shaders
    gl::shader cs_clusterDyn(GL_COMPUTE_SHADER, SHADER_PATH "cluster/clusterdyn.comp");

    // Render shaders
    gl::shader vs_renderCluster(GL_VERTEX_SHADER, SHADER_PATH "cluster/cluster.vert");
    vs_renderCluster.setIncludeSrc("math.glsl", SHADER_PATH "lib/math.glsl");
    vs_renderCluster.setIncludeSrc("complex.glsl", SHADER_PATH "geodesic/complex.glsl");
    vs_renderCluster.setIncludeSrc("elliptic.glsl", SHADER_PATH "geodesic/elliptic.glsl");
    vs_renderCluster.setIncludeSrc("geodesic.glsl", SHADER_PATH "geodesic/geodesic.glsl");
    gl::shader fs_renderCluster(GL_FRAGMENT_SHADER, SHADER_PATH "cluster/cluster.frag");
    gl::shader vs_renderBg(GL_VERTEX_SHADER, SHADER_PATH "bg/bg.vert");
    gl::shader fs_renderBg(GL_FRAGMENT_SHADER, SHADER_PATH "bg/bg.frag");
    fs_renderBg.setIncludeSrc("math.glsl", SHADER_PATH "lib/math.glsl");
    fs_renderBg.setIncludeSrc("complex.glsl", SHADER_PATH "geodesic/complex.glsl");
    fs_renderBg.setIncludeSrc("elliptic.glsl", SHADER_PATH "geodesic/elliptic.glsl");
    fs_renderBg.setIncludeSrc("geodesic.glsl", SHADER_PATH "geodesic/geodesic.glsl");

    // Build programs
    std::string log;

    // Cluster generation programs
    prog_rngUniform = new gl::program();
    prog_rngUniform->attachShader(cs_rngUniform);
    if (!prog_rngUniform->build(&log)) {
      std::cerr << "Rng uniform program" << std::endl;
      std::cerr << "═══════════════════" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    prog_rngGauss = new gl::program();
    prog_rngGauss->attachShader(cs_rngGauss);
    if (!prog_rngGauss->build(&log)) {
      std::cerr << "Rng gauss program" << std::endl;
      std::cerr << "═════════════════" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    prog_clusterpos = new gl::program();
    prog_clusterpos->attachShader(cs_clusterpos);
    if (!prog_clusterpos->build(&log)) {
      std::cerr << "Cluster position program" << std::endl;
      std::cerr << "════════════════════════" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    prog_clustercol = new gl::program();
    prog_clustercol->attachShader(cs_clustercol);
    if (!prog_clustercol->build(&log)) {
      std::cerr << "Cluster color program" << std::endl;
      std::cerr << "═════════════════════" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Cluster dynamik programs
    prog_clusterdyn = new gl::program();
    prog_clusterdyn->attachShader(cs_clusterDyn);
    if (!prog_clusterdyn->build(&log)) {
      std::cerr << "Cluster dynamik program" << std::endl;
      std::cerr << "═══════════════════════" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Render programs
    prog_renderCluster = new gl::program();
    prog_renderCluster->attachShader(vs_renderCluster);
    prog_renderCluster->attachShader(fs_renderCluster);
    if (!prog_renderCluster->build(&log)) {
      std::cerr << "Cluster render program" << std::endl;
      std::cerr << "══════════════════════" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    prog_renderBg = new gl::program();
    prog_renderBg->attachShader(vs_renderBg);
    prog_renderBg->attachShader(fs_renderBg);
    if (!prog_renderBg->build(&log)) {
      std::cerr << "Background render program" << std::endl;
      std::cerr << "═════════════════════════" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Initialize Background rendering Buffers
    glGenVertexArrays(1, &glBgVerts);
    glGenBuffers(1, &glBgPosBuf);
    glGenBuffers(1, &glBgElementBuf);

    glBindVertexArray(glBgVerts);

    std::vector<glm::vec2> pos = {
      { -1.0f, -1.0f },
      { 1.0f, -1.0f },
      { -1.0f, 1.0f },
      { 1.0f, 1.0f }
    };
    glBindBuffer(GL_ARRAY_BUFFER, glBgPosBuf);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec2), pos.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, (void*)0);

    std::vector<uint> elements = {
      0, 2, 1,
      1, 2, 3
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBgElementBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(uint), elements.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    // Initialize background textures
    glGenTextures(1, &glBgTex);
    glBindTexture(GL_TEXTURE_2D, glBgTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    config = CONFIG_INIT;
  }

  glm::vec3 getCameraPos() {
    assert(isInit());
    return cam.pos;
  }
  glm::vec3 getCameraLookDir() {
    return cam.lookDir;
  }
  glm::vec3 getCameraUpDir() {
    return cam.upDir;
  }
  float getCameraFov() {
    return cam.fov;
  }
  float getCameraAspect() {
    return cam.aspect;
  }
  float getCameraZNear() {
    return cam.zNear;
  }
  float getCameraZFar() {
    return cam.zFar;
  }
  void setCamera(glm::vec3 pos, glm::vec3 lookDir, glm::vec3 upDir,
    float fov, float aspect, float zNear, float zFar) {
    assert(isInit());
    setCameraPos(pos);
    setCameraLookDir(lookDir);
    setCameraUpDir(upDir);
    setCameraFov(fov);
    setCameraAspect(aspect);
    setCameraZNear(zNear);
    setCameraZFar(zFar);
  }
  void setCameraView(glm::vec3 pos, glm::vec3 lookDir) {
    assert(isInit());
    setCameraPos(pos);
    setCameraLookDir(lookDir);
  }
  void setCameraPos(glm::vec3 pos) {
    assert(isInit());
    cam.pos = pos;
  }
  void setCameraLookDir(glm::vec3 lookDir) {
    assert(isInit());
    cam.lookDir = lookDir;
  }
  void setCameraUpDir(glm::vec3 upDir) {
    assert(isInit());
    cam.upDir = upDir;
  }
  void setCameraFov(float fov) {
    assert(isInit());
    cam.fov = fov;
  }
  void setCameraAspect(float aspect) {
    assert(isInit());
    cam.aspect = aspect;
  }
  void setCameraZNear(float zNear) {
    assert(isInit());
    cam.zNear = zNear;
  }
  void setCameraZFar(float zFar) {
    assert(isInit());
    cam.zFar = zFar;
  }
  void updateCamera() {
    assert(isInit());
    
    P = glm::perspective(cam.fov, cam.aspect, cam.zNear, cam.zFar);
    P_I = glm::mat4(0.0f);
    P_I[0][0] = 1.0f / P[0][0];
    P_I[1][1] = 1.0f / P[1][1];

    if (!hasCamera())
      config = (slConfig)(config | CONFIG_HAS_CAM);
    updateCameraView();
  }
  void updateCameraView() {
    assert(isInit() && hasCamera());

    V = glm::lookAt(cam.pos, cam.pos + cam.lookDir, cam.upDir);
    glm::mat4 PV = P * V;

    glUseProgram(prog_renderBg->id);
    glUniform4fv(0, 1, &cam.pos[0]);
    glUniformMatrix2fv(1, 1, false, &P_I[0][0]);
    glUniformMatrix4fv(2, 1, true, &V[0][0]);
    glUseProgram(0);

    glUseProgram(prog_renderCluster->id);
    glUniformMatrix4fv(0, 1, false, &PV[0][0]);
    glUniform4fv(1, 1, &cam.pos[0]);
    glUseProgram(0);

    if (!hasCamera())
      config = (slConfig)(config | CONFIG_HAS_CAM);
  }

  void setBackgroundTex(uint width, uint height, std::vector<char>* data) {
    assert(isInit());
    setBackgroundTexSize(width, height);
    setBackgroundTexData(data);
  }
  void setBackgroundTexSize(uint width, uint height) {
    assert(isInit());
    bgTexWidth = width;
    bgTexHeight = height;
  }
  void setBackgroundTexData(std::vector<char>* data) {
    assert(isInit());
    bgTexData = data;
  }
  void updateBackgroundTex() {
    assert(isInit());

    // Set background texture to passed image
    glBindTexture(GL_TEXTURE_2D, glBgTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, bgTexWidth, bgTexHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, bgTexData->data());

    config = (slConfig)(config | CONFIG_HAS_BG_TEX);
  }
  void updateBackgroundTexData() {
    assert(isInit() && hasBgTex());

    // Set background texture to passed image
    glBindTexture(GL_TEXTURE_2D, glBgTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bgTexWidth, bgTexHeight, GL_RGBA, GL_UNSIGNED_BYTE, bgTexData->data());
  }

  void renderClassic() {
    assert(isInit() && hasCamera() && hasBgTex());
    glFinish();

    glUseProgram(prog_renderBg->id);
    glBindVertexArray(glBgVerts);
    
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, glBgTex);

    // Render Background
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glUseProgram(0);

    glFinish();
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // Render Particles
    glUseProgram(prog_renderCluster->id);
    glMemoryBarrier(GL_UNIFORM_BARRIER_BIT);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    for (int i = 0; i < (int)glClusterVerts.size(); i++) {
      glBindVertexArray(glClusterVerts[i]);
      glFinish();

      // glPointSize(3.0);

      glUniform1i(2, 1);
      glUniform1ui(3, 0);
      glDrawArrays(GL_POINTS, 0, (int)(nClusterVerts[i]));

      glUniform1ui(3, 1);
      glDrawArrays(GL_POINTS, 0, (int)(nClusterVerts[i]));

      glUniform1ui(2, 0);
      glUniform1ui(3, 0);
      glDrawArrays(GL_POINTS, 0, (int)(nClusterVerts[i]));

      glUniform1ui(3, 1);
      glDrawArrays(GL_POINTS, 0, (int)(nClusterVerts[i]));

      glBindVertexArray(0);
    }
    glUseProgram(0);
  }
  void translateGlobalTime(float t) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(msgCallback, 0);

    glFinish();

    float dt = t - globalTime;
    glUseProgram(prog_clusterdyn->id);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    for (uint i = 0; i < glClusterVerts.size(); ++i) {
      glUniform1ui(0, nClusterVerts[i]);
      glUniform1f(1, dt);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glClusterPosBufs[i]);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, glClusterVelBufs[i]);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, glClusterLBufs[i]);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, glClusterEBufs[i]);
      glFinish();

      glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);
    }
    glUseProgram(0);
    globalTime = t;
  }

  void createEllipticCluster(uint nParticles, float a, float b, glm::vec3 n, float dr, float dz,
    std::vector<glm::vec4> palette, std::vector<float> blurSizes) {
    assert(isInit());
    
    std::cout << "Compute cluster... ";

    glFinish();
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    
    // Initialize cluster buffers
    GLuint clusterVerts;
    GLuint posBuf;
    GLuint colorBuf;
    GLuint velBuf;
    GLuint LBuf;
    GLuint EBuf;
    glGenVertexArrays(1, &clusterVerts);
    glGenBuffers(1, &posBuf);
    glGenBuffers(1, &colorBuf);
    
    glBindVertexArray(clusterVerts);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuf);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, false, 0, (void*)0);
    glBindVertexArray(0);

    glGenBuffers(1, &velBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &LBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, LBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles * sizeof(float), NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &EBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, EBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles * sizeof(float), NULL, GL_STATIC_DRAW);

    // Initialize buffers
    GLuint uPosSamplesBuf;
    GLuint uColSamplesBuf;
    GLuint nPosSamplesBuf;
    GLuint paletteBuf;
    GLuint blurSizesBuf;
    glGenBuffers(1, &uPosSamplesBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uPosSamplesBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles * sizeof(float), NULL, GL_STREAM_READ);
    glGenBuffers(1, &uColSamplesBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uColSamplesBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles * sizeof(float), NULL, GL_STREAM_READ);
    glGenBuffers(1, &nPosSamplesBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nPosSamplesBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 5 * nParticles * sizeof(float), NULL, GL_STREAM_READ);
    glGenBuffers(1, &paletteBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paletteBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, palette.size() * sizeof(glm::vec4), palette.data(), GL_STREAM_READ);
    glGenBuffers(1, &blurSizesBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blurSizesBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, blurSizes.size() * sizeof(float), blurSizes.data(), GL_STREAM_READ);

    // Generate uniform samples
    uint64 off_ = getRngOff();
    glm::uvec2 off = glm::uvec2((uint)((off_ >> 8 * sizeof(uint)) & UINT_MAX), (uint)(off_ & UINT_MAX));
    glUseProgram(prog_rngUniform->id);
    glUniform1ui(0, nParticles);
    glUniform2ui(1, off.x, off.y);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, uPosSamplesBuf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);

    off_ = getRngOff();
    off = glm::uvec2((uint)((off_ >> 8 * sizeof(uint)) & UINT_MAX), (uint)(off_ & UINT_MAX));
    glUniform2ui(1, off.x, off.y);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, uColSamplesBuf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);
    glUseProgram(0);

    // Generate gaussian samples
    off_ = getRngOff();
    off = glm::uvec2((uint)((off_ >> 8 * sizeof(uint)) & UINT_MAX), (uint)(off_ & UINT_MAX));
    glUseProgram(prog_rngGauss->id);
    glUniform1ui(0, 5 * nParticles);
    glUniform2ui(1, off.x, off.y);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nPosSamplesBuf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);
    glUseProgram(0);

    glFinish();
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    float eps = sqrt(1 - b * b / (a * a));
    glm::mat4 rot = glm::rotate(n.z / glm::length(n), glm::vec3(-n.y, n.x, 0.0));
    glUseProgram(prog_clusterpos->id);
    glUniform1ui(0, nParticles);
    glUniform1f(1, b);
    glUniform1f(2, eps);
    glUniformMatrix4fv(3, 1, false, &rot[0][0]);
    glUniform1f(4, dr);
    glUniform1f(5, dz);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, uPosSamplesBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, nPosSamplesBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, posBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, velBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, LBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, EBuf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);
    glUseProgram(0);

    glUseProgram(prog_clustercol->id);
    glUniform1ui(0, nParticles);
    glUniform1ui(1, palette.size());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, paletteBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blurSizesBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, uColSamplesBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, colorBuf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);
    glUseProgram(0);

    nClusterVerts.push_back(nParticles);
    glClusterVerts.push_back(clusterVerts);
    glClusterPosBufs.push_back(posBuf);
    glClusterColBufs.push_back(colorBuf);
    glClusterVelBufs.push_back(velBuf);
    glClusterLBufs.push_back(LBuf);
    glClusterEBufs.push_back(EBuf);

    glFinish();
    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
    float dt1 = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() * 1.0e-9;
    float dt2 = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count() * 1.0e-9;
    float dt = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t1).count() * 1.0e-9;
    std::cout << "Done" << std::endl;
    std::cout << "Sample computation time for " << nParticles << " particles: " << dt1 << " s" << std::endl;
    std::cout << "Cluster computation time for " << nParticles << " particles: " << dt2 << " s" << std::endl;
    std::cout << "Total computation time for " << nParticles << " particles: " << dt << " s" << std::endl;
    std::cout << std::endl;
  }
  void clearClusters() {
    // Clear particle GPU buffers
    for (int i = 0; i < (int)glClusterVerts.size(); i++) {
      glDeleteBuffers(1, &glClusterPosBufs[i]);
      glDeleteBuffers(1, &glClusterColBufs[i]);
      glDeleteVertexArrays(1, &glClusterVerts[i]);
    }
    nClusterVerts.clear();
    glClusterVerts.clear();
    glClusterPosBufs.clear();
    glClusterColBufs.clear();
  }
  void selectCluster(int index) {
    sCluster = index;
  }
  void deselectClusters() {
    sCluster = -1;
  }

  void close() {
    assert((config & CONFIG_INIT) != 0);

    clearClusters();

    glDeleteTextures(1, &glBgTex);
    glDeleteTextures(1, &glBgRenderTex);

    glDeleteBuffers(1, &glBgPosBuf);
    glDeleteBuffers(1, &glBgElementBuf);
    glDeleteVertexArrays(1, &glBgVerts);

    delete prog_rngUniform;
    delete prog_rngGauss;
    delete prog_clusterpos;
    delete prog_clustercol;
    delete prog_renderCluster;
    delete prog_renderBg;

    config = CONFIG_CLOSED;
  }

  uint64 getRngOff() {
    uint64 dt = time(0);
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    t1 = std::chrono::floor<std::chrono::seconds>(t1);
    return dt * 1000000000ULL + (uint64)std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
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
}
