#define _USE_MATH_DEFINES

#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

#include "tmeas.hpp"
#include "renderwidget.hpp"

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent) {
  // format.setRenderableType(QSurfaceFormat::RenderableType::OpenGLES);
  format.setVersion(3, 2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  setFormat(format);
}
RenderWidget::~RenderWidget() {
  sl::close();
}

bool RenderWidget::isCameraMoving() {
  return cameraIsMoving;
}
QPoint RenderWidget::getCameraGrabPoint() {
  return cameraGrabPoint;
}

// Signals
void RenderWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton) {
    cameraGrabPoint = event->pos();
    cameraGrabPos = sl::getCameraPos();
    cameraIsMoving = true;
  }
}
void RenderWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton) {
    cameraIsMoving = false;
  }
}
void RenderWidget::wheelEvent(QWheelEvent* event) {
  glm::vec3 pos = sl::getCameraPos(); 
  pos *= powf(facPerTurn, -event->delta() / 2880.0f );
  sl::setCameraPos(pos);
  sl::updateCameraView();
  update();
}

// Slots
void RenderWidget::cameraMove(QPoint mouseVector) {
  if (!cameraIsMoving)
    return;

  float r = glm::length(cameraGrabPos);
  float theta = acosf(cameraGrabPos.z / r);
  float phi = atan2f(cameraGrabPos.y, cameraGrabPos.x);

  theta += -2.0f * M_PIf32 * mouseVector.y() / dotsPerTurn;
  phi += -2.0f * M_PIf32 * mouseVector.x() / dotsPerTurn;
  glm::vec3 pos = {
    r * sinf(theta) * cosf(phi),
    r * sinf(theta) * sinf(phi),
    r * cosf(theta)
  };
  sl::setCameraView(pos, -pos);
  sl::updateCameraView();
  update();
}

// Events
void RenderWidget::initializeGL() {
  makeCurrent();

  sl::init();
  
  // Set background
  QImage bgTex = QImage(":/textures/bg1.jpg");
  std::vector<char> bgTexData(bgTex.bits(), bgTex.bits() + bgTex.sizeInBytes());
  sl::setBackgroundTex((uint)bgTex.width(), (uint)bgTex.height(), &bgTexData);

  // Generate cluster
  uint nParticles = 250000;
  float a = 20.0f;
  float b = 15.0f;
  float nx = 1.0f;
  float ny = -1.0f;
  float nz = 1.0f;
  float dr = 1.0f;
  float dz = 0.5f;
  glm::vec3 n = { nx, ny, nz };
  std::vector<glm::vec4> palette = { { 1.00f, 0.30f, 0.00f, 0.1f } };
  std::vector<float> blurSizes = { 1.00f };
  sl::createEllipticCluster(nParticles, a, b, n, dr, dz, palette, blurSizes);

  // Set camera
  glm::vec3 pos = { 30.0f, 0.0f, 0.0f };
  glm::vec3 upDir = { 0.0f, 0.0f, 1.0f };
  float fov = glm::radians(60.0f);
  float aspect = float(width()) / float(height());
  float zNear = 0.1f;
  float zFar = 100.0f;
  sl::setCamera(pos, -pos, upDir, fov, aspect, zNear, zFar);

  // Update data
  sl::updateBackgroundTex();
  sl::updateCamera();
  update();

  emit glInitialized();
}
void RenderWidget::resizeGL(int w, int h) {
  sl::setCameraAspect(float(w) / float(h));
  sl::updateCamera();
}
void RenderWidget::paintGL() {
  sl::renderClassic();
}
