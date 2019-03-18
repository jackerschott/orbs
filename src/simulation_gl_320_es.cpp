#define _USE_MATH_DEFINES
#define PI float(M_PI)
#define PI_2 float(M_PI_2)

// This improvisational install path is only working if the binary is executed from workspace/project folder!
#define INSTALL_PATH "./"
#define SHADER_PATH INSTALL_PATH "src/shader_320_es/"
#define TMP_PATH INSTALL_PATH "tmp/"

#include <chrono>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <time.h>
#include <utility>
#include <vector>

#include "glinc.hpp"
#include "simulation_gl_320_es.hpp"

namespace sl {
  // Physics data
  float rs;
  camera observer;
  bool isRel = false;

  // GPU Rendering programs
  GLuint bg_prog;
  GLuint bg_vs;
  GLuint bg_fs;

  GLuint clusterRender_prog;
  GLuint cluster_vs;
  GLuint cluster_fs;

  // GPU Computation programs
  GLuint uniformRng_prog;
  GLuint uniformRng_cs;
  GLuint gaussRng_prog;
  GLuint gaussRng_cs;
  GLuint clusterPos_prog;
  GLuint clusterPos_cs;
  GLuint clusterCol_prog;
  GLuint clusterCol_cs;

  // GPU buffers
  GLuint glBg;
  GLuint glBgTexture;
  GLuint glBgPosBuf;
  GLuint glBgTexCoordBuf;

  std::vector<uint> nClusterVerts;
  std::vector<GLuint> glClusterVerts;
  std::vector<GLuint> glClusterPosBufs;
  std::vector<GLuint> glClusterColBufs;
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
  glm::vec2 bgTexCoord[6];
  glm::mat4 perspv;
  GLint viewPj;

  // Organisation Details
  slConfig config = CONFIG_CLOSED;

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

    glUseProgram(clusterRender_prog);
    glUniformMatrix4fv(viewPj, 1, false, &vp[0][0]);
    glUseProgram(0);
  }

  void init(float _rs) {
    assert(config == CONFIG_CLOSED);

    rs = _rs;

    // Initialize OpenGL
    glewInit();
    #if _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(msgCallback, 0);
    #endif
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build rendering programs
    buildClusterProg();
    buildBgProg();

    glUseProgram(clusterRender_prog);
    viewPj = glGetUniformLocation(clusterRender_prog, "viewPj");
    glUseProgram(0);

    // Build computation programs
    buildUniformRngCompProg();
    buildGaussRngCompProg();
    buildClusterPosCompProg();
    buildClusterColCompProg();

    // Initialize Background rendering Buffers
    glGenVertexArrays(1, &glBg);
    glBindVertexArray(glBg);

    glGenBuffers(1, &glBgPosBuf);
    glBindBuffer(GL_ARRAY_BUFFER, glBgPosBuf);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);

    glGenBuffers(1, &glBgTexCoordBuf);
    glBindBuffer(GL_ARRAY_BUFFER, glBgTexCoordBuf);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, (void*)0);

    glBindVertexArray(0);

    // Initialize background texture
    glGenTextures(1, &glBgTexture);
    glBindTexture(GL_TEXTURE_2D, glBgTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    config = CONFIG_INIT;
  }

  void createEllipticCluster(uint nParticles, float a, float b, vector n, float dr, float dz,
    uint nColors, color* palette, float* blurSizes) {
    glFinish();
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    
    // Initialize cluster buffers
    GLuint clusterVerts;
    GLuint posBuf;
    GLuint colorBuf;
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

    // Initialize buffers
    GLuint uSamples1Buf;
    GLuint uSamples2Buf;
    GLuint nSamples1Buf;
    GLuint nSamples2Buf;
    GLuint paletteBuf;
    GLuint blurSizesBuf;
    glGenBuffers(1, &uSamples1Buf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uSamples1Buf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles * sizeof(float), NULL, GL_STREAM_READ);
    glGenBuffers(1, &uSamples2Buf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uSamples2Buf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles * sizeof(float), NULL, GL_STREAM_READ);
    glGenBuffers(1, &nSamples1Buf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nSamples1Buf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles * sizeof(float), NULL, GL_STREAM_READ);
    glGenBuffers(1, &nSamples2Buf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nSamples2Buf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles * sizeof(float), NULL, GL_STREAM_READ);
    glGenBuffers(1, &paletteBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paletteBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nColors * sizeof(glm::vec4), palette, GL_STREAM_READ);
    glGenBuffers(1, &blurSizesBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blurSizesBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nColors * sizeof(float), blurSizes, GL_STREAM_READ);

    // Generate uniform samples
    uint64 off_ = getRngOff();
    glm::uvec2 off = glm::uvec2((uint)((off_ >> 8 * sizeof(uint)) & UINT_MAX), (uint)(off_ & UINT_MAX));
    glUseProgram(uniformRng_prog);
    glUniform1ui(0, nParticles);
    glUniform2ui(1, off.x, off.y);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, uSamples1Buf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);

    off_ = getRngOff();
    off = glm::uvec2((uint)((off_ >> 8 * sizeof(uint)) & UINT_MAX), (uint)(off_ & UINT_MAX));
    glUniform2ui(1, off.x, off.y);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, uSamples2Buf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);
    glUseProgram(0);

    // Generate gaussian samples
    off_ = getRngOff();
    off = glm::uvec2((uint)((off_ >> 8 * sizeof(uint)) & UINT_MAX), (uint)(off_ & UINT_MAX));
    glUseProgram(gaussRng_prog);
    glUniform1ui(0, nParticles);
    glUniform2ui(1, off.x, off.y);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nSamples1Buf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);

    off_ = getRngOff();
    off = glm::uvec2((uint)((off_ >> 8 * sizeof(uint)) & UINT_MAX), (uint)(off_ & UINT_MAX));
    glUniform2ui(1, off.x, off.y);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nSamples2Buf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);
    glUseProgram(0);

    glFinish();
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    float eps = sqrt(1 - b * b / (a * a));
    glm::mat4 rot = glm::rotate(n.z / glm::length(n), glm::vec3(-n.y, n.x, 0.0));
    glUseProgram(clusterPos_prog);
    glUniform1ui(0, nParticles);
    glUniform1f(1, b);
    glUniform1f(2, eps);
    glUniformMatrix4fv(3, 1, false, &rot[0][0]);
    glUniform1f(4, dr);
    glUniform1f(5, dz);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, uSamples1Buf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, nSamples1Buf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, nSamples2Buf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, posBuf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);
    glUseProgram(0);

    glUseProgram(clusterCol_prog);
    glUniform1ui(0, nParticles);
    glUniform1ui(1, nColors);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, paletteBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blurSizesBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, uSamples2Buf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, colorBuf);
    glDispatchCompute(0x10 / 4, 0x10 / 4, 0x10 / 4);
    glUseProgram(0);

    nClusterVerts.push_back(nParticles);
    glClusterVerts.push_back(clusterVerts);
    glClusterPosBufs.push_back(posBuf);
    glClusterColBufs.push_back(colorBuf);

    glFinish();
    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
    float dt1 = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() * 1.0e-9;
    float dt2 = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count() * 1.0e-9;
    float dt = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t1).count() * 1.0e-9;
    std::cout << "Sample computation time for " << nParticles << " particles: " << dt1 << " s" << std::endl;
    std::cout << "Cluster computation time for " << nParticles << " particles: " << dt2 << " s" << std::endl;
    std::cout << "Total computation time for " << nParticles << " particles: " << dt << " s" << std::endl;
    std::cout << std::endl;

    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, uSamples1Buf);
    // float* uSamples1 = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, nParticles * sizeof(float), GL_MAP_READ_BIT);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, uSamples2Buf);
    // float* uSamples2 = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, nParticles * sizeof(float), GL_MAP_READ_BIT);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, nSamples1Buf);
    // float* nSamples1 = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, nParticles * sizeof(float), GL_MAP_READ_BIT);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, nSamples2Buf);
    // float* nSamples2 = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, nParticles * sizeof(float), GL_MAP_READ_BIT);

    // std::ofstream out;
    // out.open("tmp/samples.txt");
    // for (uint i = 0; i < nParticles; ++i) {
    //   out << uSamples1[i] << '\t' << uSamples2[i] << '\t' << nSamples1[i] << '\t' << nSamples2[i] << std::endl;
    // }
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

  void setObserverCamera(camera _observer) {
    assert((config & CONFIG_INIT) != 0 && (config & CONFIG_HAS_CAMERA) == 0);

    // Normalize Vectors and set observer camera
    _observer.lookDir = glm::normalize(_observer.lookDir);
    _observer.upDir = glm::normalize(_observer.upDir);
    observer = _observer;

    // Set View Projection Matrix
    perspv = glm::perspective(observer.fov, observer.aspect, observer.zNear, observer.zFar);
    glm::mat4 vp = perspv * glm::lookAt(observer.pos, observer.pos + observer.lookDir, observer.upDir);
    glUseProgram(clusterRender_prog);
    glUniformMatrix4fv(viewPj, 1, false, &vp[0][0]);
    glUseProgram(0);

    config = (slConfig)(config | CONFIG_HAS_CAMERA);
  }
  void setBackgroundTex(UNUSED uint sData, byte* data, uint width, uint height, UNUSED uint bpp) {
    assert((config & CONFIG_INIT) != 0 && (config & CONFIG_HAS_BG_TEX) == 0);

    // Set background shader inputs
    glBindVertexArray(glBg);

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

    glUseProgram(clusterRender_prog);
    glUniformMatrix4fv(viewPj, 1, false, &vp[0][0]);
    glUseProgram(0);
  }

  void updateParticlesClassic(UNUSED float time) {

  }
  void updateParticlesRelatvistic(UNUSED float time) {
    
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

    glUseProgram(bg_prog);
    glBindVertexArray(glBg);
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
    glUseProgram(clusterRender_prog);
    for (int i = 0; i < (int)glClusterVerts.size(); i++) {
      glBindVertexArray(glClusterVerts[i]);
      glDrawArrays(GL_POINTS, 0, (int)(nClusterVerts[i]));
      glBindVertexArray(0);
    }
    glUseProgram(0);

    glFinish();

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
    glDeleteVertexArrays(1, &glBg);

    glDetachShader(clusterRender_prog, cluster_vs);
    glDeleteShader(cluster_vs);
    glDetachShader(clusterRender_prog, cluster_fs);
    glDeleteShader(cluster_fs);
    glDeleteProgram(clusterRender_prog);

    config = CONFIG_CLOSED;
  }

  uint64 getRngOff() {
    uint64 dt = time(0);
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
    if (!readShaderSrc(SHADER_PATH "cluster.vert", &src, &len)) {
      std::cerr << SHADER_PATH "cluster.vert" << " could not be opened." << std::endl;
      throw;
    }
    glShaderSource(cluster_vs, 1, &src, &len);
    delete[] src;

    cluster_fs = glCreateShader(GL_FRAGMENT_SHADER);
    if (!readShaderSrc(SHADER_PATH "cluster.frag", &src, &len)) {
      std::cerr << SHADER_PATH "cluster.frag" << "could not be opened." << std::endl;
      throw;
    }
    glShaderSource(cluster_fs, 1, &src, &len);
    delete[] src;

    // Compile shaders
    clusterRender_prog = glCreateProgram();
    glCompileShader(cluster_vs);
    glGetShaderiv(cluster_vs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(cluster_vs, sizeof(log), NULL, log);
      std::cerr << "Cluster vertex shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(clusterRender_prog, cluster_vs);

    glCompileShader(cluster_fs);
    glGetShaderiv(cluster_fs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(cluster_fs, sizeof(log), NULL, log);
      std::cerr << "Cluster fragment shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(clusterRender_prog, cluster_fs);

    // Bind attribute locations
    glBindAttribLocation(clusterRender_prog, 0, "pos");
    glBindAttribLocation(clusterRender_prog, 1, "color");

    // Link Program
    glLinkProgram(clusterRender_prog);
    glGetProgramiv(clusterRender_prog, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(clusterRender_prog, sizeof(log), NULL, log);
      std::cerr << "Cluster program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Validate Program
    glValidateProgram(clusterRender_prog);
    glGetProgramiv(clusterRender_prog, GL_VALIDATE_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(clusterRender_prog, sizeof(log), NULL, log);
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
    if (!readShaderSrc(SHADER_PATH "bg.vert", &src, &len)) {
      std::cerr << SHADER_PATH "bg.vert" << " could not be opened." << std::endl;
      throw;
    }
    glShaderSource(bg_vs, 1, &src, &len);
    delete[] src;

    bg_fs = glCreateShader(GL_FRAGMENT_SHADER);
    if (!readShaderSrc(SHADER_PATH "bg.frag", &src, &len)) {
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

  void buildUniformRngCompProg() {
    char* src;
    int len;
    int success;
    char log[0x400];

    // Load shader includes
    if (!readShaderSrc(SHADER_PATH "uint64.comp", &src, &len)) {
      std::cerr << SHADER_PATH "uint64.comp" << " could not be opened." << std::endl;
      throw;
    }
    glinc::addIncludeSrc("uint64.comp", src);
    delete[] src;
    if (!readShaderSrc(SHADER_PATH "skip_mwc.comp", &src, &len)) {
      std::cerr << SHADER_PATH "skip_mwc.comp" << " could not be opened." << std::endl;
      throw;
    }
    glinc::addIncludeSrc("skip_mwc.comp", src);
    delete[] src;
    if (!readShaderSrc(SHADER_PATH "mwc64x_rng.comp", &src, &len)) {
      std::cerr << SHADER_PATH "mwc64x_rng.comp" << " could not be opened." << std::endl;
      throw;
    }
    glinc::addIncludeSrc("mwc64x_rng.comp", src);
    delete[] src;

    // Create shader
    uniformRng_cs = glCreateShader(GL_COMPUTE_SHADER);
    if (!readShaderSrc(SHADER_PATH "uniform_rng.comp", &src, &len)) {
      std::cerr << SHADER_PATH "uniform_rng.comp" << "could not be opened." << std::endl;
      throw;
    }
    char* src_ = (char*)glinc::insertIncludes(src);
    int len_ = strlen(src_);
    writeShaderSrc(TMP_PATH "uniform_rng.comp", src_, len_);
    glShaderSource(uniformRng_cs, 1, &src_, &len_);
    delete[] src;

    // Compile shaders
    uniformRng_prog = glCreateProgram();
    glCompileShader(uniformRng_cs);
    glGetShaderiv(uniformRng_cs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(uniformRng_cs, sizeof(log), NULL, log);
      std::cerr << "Uniform rng compute shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(uniformRng_prog, uniformRng_cs);

    // Link Program
    glLinkProgram(uniformRng_prog);
    glGetProgramiv(uniformRng_prog, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(uniformRng_prog, sizeof(log), NULL, log);
      std::cerr << "Uniform rng compute program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Validate Program
    glValidateProgram(uniformRng_prog);
    glGetProgramiv(uniformRng_prog, GL_VALIDATE_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(uniformRng_prog, sizeof(log), NULL, log);
      std::cerr << "Uniform rng compute program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
  }
  void buildGaussRngCompProg() {
    char* src;
    int len;
    int success;
    char log[0x400];

    // Load shader includes
    if (!readShaderSrc(SHADER_PATH "uint64.comp", &src, &len)) {
      std::cerr << SHADER_PATH "uint64.comp" << " could not be opened." << std::endl;
      throw;
    }
    glinc::addIncludeSrc("uint64.comp", src);
    delete[] src;
    if (!readShaderSrc(SHADER_PATH "skip_mwc.comp", &src, &len)) {
      std::cerr << SHADER_PATH "skip_mwc.comp" << " could not be opened." << std::endl;
      throw;
    }
    glinc::addIncludeSrc("skip_mwc.comp", src);
    delete[] src;
    if (!readShaderSrc(SHADER_PATH "mwc64x_rng.comp", &src, &len)) {
      std::cerr << SHADER_PATH "mwc64x_rng.comp" << " could not be opened." << std::endl;
      throw;
    }
    glinc::addIncludeSrc("mwc64x_rng.comp", src);
    delete[] src;
    if (!readShaderSrc(SHADER_PATH "rng_tables.comp", &src, &len)) {
      std::cerr << SHADER_PATH "rng_tables.comp" << " could not be opened." << std::endl;
    }
    glinc::addIncludeSrc("rng_tables.comp", src);
    delete[] src;

    // Create shader
    gaussRng_cs = glCreateShader(GL_COMPUTE_SHADER);
    if (!readShaderSrc(SHADER_PATH "gauss_rng.comp", &src, &len)) {
      std::cerr << SHADER_PATH "gauss_rng.comp" << "could not be opened." << std::endl;
      throw;
    }
    char* src_ = glinc::insertIncludes(src);
    int len_ = strlen(src_);
    writeShaderSrc(TMP_PATH "gauss_rng.comp", src_, len_);
    glShaderSource(gaussRng_cs, 1, &src_, &len_);
    delete[] src;

    // Compile shaders
    gaussRng_prog = glCreateProgram();
    glCompileShader(gaussRng_cs);
    glGetShaderiv(gaussRng_cs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(gaussRng_cs, sizeof(log), NULL, log);
      std::cerr << "Gauss rng compute shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(gaussRng_prog, gaussRng_cs);

    // Link Program
    glLinkProgram(gaussRng_prog);
    glGetProgramiv(gaussRng_prog, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(gaussRng_prog, sizeof(log), NULL, log);
      std::cerr << "Gauss rng compute program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Validate Program
    glValidateProgram(gaussRng_prog);
    glGetProgramiv(gaussRng_prog, GL_VALIDATE_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(gaussRng_prog, sizeof(log), NULL, log);
      std::cerr << "Gauss rng compute program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
  }
  void buildClusterPosCompProg() {
    char* src;
    int len;
    int success;
    char log[0x400];

    // Create shader
    clusterPos_cs = glCreateShader(GL_COMPUTE_SHADER);
    if (!readShaderSrc(SHADER_PATH "clusterpos.comp", &src, &len)) {
      std::cerr << SHADER_PATH "clusterpos.comp" << "could not be opened." << std::endl;
      throw;
    }
    glShaderSource(clusterPos_cs, 1, &src, &len);
    delete[] src;

    // Compile shaders
    clusterPos_prog = glCreateProgram();
    glCompileShader(clusterPos_cs);
    glGetShaderiv(clusterPos_cs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(clusterPos_cs, sizeof(log), NULL, log);
      std::cerr << "Cluster position compute shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(clusterPos_prog, clusterPos_cs);

    // Link Program
    glLinkProgram(clusterPos_prog);
    glGetProgramiv(clusterPos_prog, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(clusterPos_prog, sizeof(log), NULL, log);
      std::cerr << "Cluster position compute program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Validate Program
    glValidateProgram(clusterPos_prog);
    glGetProgramiv(clusterPos_prog, GL_VALIDATE_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(clusterPos_prog, sizeof(log), NULL, log);
      std::cerr << "Cluster position compute program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
  }
  void buildClusterColCompProg() {
    char* src;
    int len;
    int success;
    char log[0x400];

    // Create shader
    clusterCol_cs = glCreateShader(GL_COMPUTE_SHADER);
    if (!readShaderSrc(SHADER_PATH "clustercol.comp", &src, &len)) {
      std::cerr << SHADER_PATH "clustercol.comp" << "could not be opened." << std::endl;
      throw;
    }
    glShaderSource(clusterCol_cs, 1, &src, &len);
    delete[] src;

    // Compile shaders
    clusterCol_prog = glCreateProgram();
    glCompileShader(clusterCol_cs);
    glGetShaderiv(clusterCol_cs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(clusterCol_cs, sizeof(log), NULL, log);
      std::cerr << "Cluster color compute shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(clusterCol_prog, clusterCol_cs);

    // Link Program
    glLinkProgram(clusterCol_prog);
    glGetProgramiv(clusterCol_prog, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(clusterCol_prog, sizeof(log), NULL, log);
      std::cerr << "Cluster color compute program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Validate Program
    glValidateProgram(clusterCol_prog);
    glGetProgramiv(clusterCol_prog, GL_VALIDATE_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(clusterCol_prog, sizeof(log), NULL, log);
      std::cerr << "Cluster color compute program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
  }

  bool readShaderSrc(const char* path, char** src, int* srcLen) {
    std::ifstream fs;
    fs.open(path);

    std::string out;
    std::string line;
    if (fs.is_open()) {
      while (std::getline(fs, line)) {
        out += line + '\n';
      }
    }
    else return false;
    fs.close();

    *srcLen = out.length();
    *src = new char[*srcLen + 1];
    strcpy(*src, out.c_str());
    return true;
  }
  void writeShaderSrc(const char* path, char* src, int srcLen) {
    std::ofstream fs;
    fs.open(path);
    fs << std::string(src, srcLen);
    fs.close();
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
