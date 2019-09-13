#ifndef SIMULATONEI_HPP
#define SIMULATONEI_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "res.hpp"

DEF_STANDARD_TYPES

struct camera;

namespace sl {
  bool isInit();
  bool isRendering();
  bool isClosed();

  void init();

  void createEllipticCluster(uint nParticles, float a, float b, glm::vec3 n, float dr, float dz,
    std::vector<glm::vec4> palette, std::vector<float> blurSizes);
  void clearClusters();
  void selectCluster(int index);
  void deselectClusters();

  glm::vec3 getCameraPos();
  glm::vec3 getCameraLookDir();
  glm::vec3 getCameraUpDir();
  float getCameraFov();
  float getCameraAspect();
  float getCameraZNear();
  float getCameraZFar();
  void setCamera(glm::vec3 pos, glm::vec3 lookDir, glm::vec3 upDir,
    float fov, float aspect, float zNear, float zFar);
  void setCameraView(glm::vec3 pos, glm::vec3 lookDir);
  void setCameraPos(glm::vec3 pos);
  void setCameraLookDir(glm::vec3 lookDir);
  void setCameraUpDir(glm::vec3 upDir);
  void setCameraFov(float fov);
  void setCameraAspect(float aspect);
  void setCameraZNear(float zNear);
  void setCameraZFar(float zFar);
  void updateCamera();
  void updateCameraView();

  void setBackgroundTex(uint width, uint height, std::vector<char>* data);
  void setBackgroundTexSize(uint width, uint height);
  void setBackgroundTexData(std::vector<char>* data);
  void updateBackgroundTex();
  void updateBackgroundTexData();

  void renderClassic();
  void translateGlobalTime(float t);

  void close();
}

struct camera {
  glm::vec3 pos;
  glm::vec3 lookDir;
  glm::vec3 upDir;
  float fov;
  float aspect;
  float zNear;
  float zFar;
};

enum slConfig {
  CONFIG_CLOSED       = 0b000,
  CONFIG_INIT         = 0b001,
  CONFIG_HAS_CAM      = 0b010,
  CONFIG_HAS_BG_TEX   = 0b100
};


#endif
