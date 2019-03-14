#define _USE_MATH_DEFINES
#define PI float(M_PI)
#define PI_2 float(M_PI_2)

#define INSTALL_PATH "/home/jona/Projects/BlackHoleSimulation/"
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

  GLuint cluster_prog;
  GLuint cluster_vs;
  GLuint cluster_fs;

  // GPU Computation programs
  GLuint cluster_comp_prog;
  GLuint cluster_cs;

  // GPU buffers
  GLuint glBg;
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
  glm::vec2 bgTexCoord[6];
  glm::mat4 perspv;
  GLint viewPj;

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
  void buildClusterProg();
  void buildBgProg();
  void buildClusterCompProg();
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

    glUseProgram(cluster_prog);
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

    glUseProgram(cluster_prog);
    viewPj = glGetUniformLocation(cluster_prog, "viewPj");
    glUseProgram(0);

    // Build computation programs
    buildClusterCompProg();

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

  void createEllipticCluster(int nParticles, float a, float b, vector n, float dr, float dz,
    int nColors, color* palette, float* blurSizes) {
    // TODO: Generate Particle Cluster with OpenGL Compute Shaders

    GLuint sampleBuf;
    glGenBuffers(1, &sampleBuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sampleBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles * sizeof(float), NULL, GL_STREAM_READ);

    ulong off = getRngOff();

    glUseProgram(cluster_comp_prog);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sampleBuf);
    glUniform1ui(0, nParticles);
    glUniform2ui(1, (uint)((off >> 8 * sizeof(uint)) & UINT_MAX), (uint)(off & UINT_MAX));
    glDispatchCompute(4096 / 64, 1, 1);
    glUseProgram(0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sampleBuf);
    float* samples = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, nParticles * sizeof(float), GL_MAP_READ_BIT);

    for (int i = 0; i < nParticles; ++i) {
      std::cout << samples[i] << std::endl;
    }
  }
  void clearClusters() {
    // Clear particle GPU buffers
    for (int i = 0; i < (int)glClusterPts.size(); i++) {
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
    glUseProgram(cluster_prog);
    glUniformMatrix4fv(viewPj, 1, false, &vp[0][0]);
    glUseProgram(0);

    config = (slConfig)(config | CONFIG_HAS_CAMERA);
  }
  void setBackgroundTex(UNUSED uint sData, byte* data, uint width, uint height, UNUSED uint bpp) {
    assert((config & CONFIG_INIT) != 0 && (config & CONFIG_HAS_BG_TEX) == 0);

    // for (int i = 0; i <= sData; i++) {
    //   if (data[i] != '\0' && data[i] != '\255') {
    //     std::cout << (int)data[i] << std::endl;
    //   }
    // }

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

    glUseProgram(cluster_prog);
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
    glUseProgram(cluster_prog);
    for (int i = 0; i < (int)glClusterPts.size(); i++) {
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
    glDeleteVertexArrays(1, &glBg);

    glDetachShader(cluster_prog, cluster_vs);
    glDeleteShader(cluster_vs);
    glDetachShader(cluster_prog, cluster_fs);
    glDeleteShader(cluster_fs);
    glDeleteProgram(cluster_prog);

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

  void buildClusterCompProg() {
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
    cluster_cs = glCreateShader(GL_COMPUTE_SHADER);
    if (!readShaderSrc(SHADER_PATH "uniform_rng.comp", &src, &len)) {
      std::cerr << SHADER_PATH "uniform_rng.comp" << "could not be opened." << std::endl;
      throw;
    }
    char* src_ = (char*)glinc::insertIncludes(src);
    int len_ = strlen(src_);
    writeShaderSrc(TMP_PATH "uniform_rng.comp", src_, len_);
    glShaderSource(cluster_cs, 1, &src_, &len_);
    delete[] src;

    // Compile shaders
    cluster_comp_prog = glCreateProgram();
    glCompileShader(cluster_cs);
    glGetShaderiv(cluster_cs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(cluster_cs, sizeof(log), NULL, log);
      std::cerr << "Cluster compute shader:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }
    glAttachShader(cluster_comp_prog, cluster_cs);

    // Link Program
    glLinkProgram(cluster_comp_prog);
    glGetProgramiv(cluster_comp_prog, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(cluster_comp_prog, sizeof(log), NULL, log);
      std::cerr << "Cluster compute program:" << std::endl;
      std::cerr << log << std::endl;
      throw;
    }

    // Validate Program
    glValidateProgram(cluster_comp_prog);
    glGetProgramiv(cluster_comp_prog, GL_VALIDATE_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(cluster_comp_prog, sizeof(log), NULL, log);
      std::cerr << "Cluster compute program:" << std::endl;
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
