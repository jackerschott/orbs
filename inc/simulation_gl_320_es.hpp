#ifndef SIMULATONEI_HPP
#define SIMULATONEI_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "res.hpp"

DEF_STANDARD_TYPES
typedef glm::vec3 vector;

struct color;
struct camera;

namespace sl {
  bool isInit();
  bool isRendering();
  bool isClosed();
  void setObserverCameraAspect(float aspect);

  void init(float _rs);

  void createEllipticCluster(uint nParticles, float a, float b, vector n, float dr, float dz,
    uint nColors, color* palette, float* blurSizes);
  void clearClusters();
  void selectCluster(int index);
  void deselectClusters();

  void setObserverCamera(camera _observer);
  void setBackgroundTex(uint sData, byte* data, uint width, uint height, uint bpp);
  void moveObserverCamera(vector pos, vector lookDir, vector upDir);

  void updateParticlesClassic(float time);
  void updateParticlesRelatvistic(float time);
  void renderClassic();
  void renderRelativistic();

  void close();
}

struct color {
  float r;
  float g;
  float b;
  float a;
};
struct camera {
  vector pos;
  vector lookDir;
  vector upDir;
  float fov;
  float aspect;
  float zNear;
  float zFar;
};

enum slConfig {
  CONFIG_CLOSED = 0b000001,
  CONFIG_INIT = 0b000010,
  CONFIG_HAS_CAMERA = 0b000100,
  CONFIG_HAS_BG_TEX = 0b001000,
  CONFIG_RENDERING = 0b010000,

  CONFIG_INIT_FOR_RENDER = 0b001110
};


#endif
