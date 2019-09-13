#define _USE_MATH_DEFINES

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

#include "renderwidget.hpp"

const float c = 303069264.0;

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent) {
  format.setVersion(3, 2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  setFormat(format);
}
RenderWidget::~RenderWidget() {
  delete timer;
  sl::close();
}

bool RenderWidget::isCameraMoving() {
  return cameraIsMoving;
}
QPoint RenderWidget::getCameraPin() {
  return cameraPin;
}

// Signals
void RenderWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton) {
    cameraUpdatePin(event->pos());
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

void RenderWidget::cameraMove(QPoint mouseVector) {
  if (!cameraIsMoving)
    return;

  float r = glm::length(cameraPinPos);
  float theta = acosf(cameraPinPos.z / r);
  float phi = atan2f(cameraPinPos.y, cameraPinPos.x);

  if (cameraIsUpright) {
    theta += -2.0f * M_PIf32 * mouseVector.y() / dotsPerTurn;
    phi += -2.0f * M_PIf32 * mouseVector.x() / dotsPerTurn;
  }
  else {
    theta += 2.0f * M_PIf32 * mouseVector.y() / dotsPerTurn;
    phi += 2.0f * M_PIf32 * mouseVector.x() / dotsPerTurn;
  }

  #define EPS std::numeric_limits<float>::epsilon()
  bool thetaOverflow = theta < 0.0f || theta > M_PIf32;
  if (thetaOverflow) {
    if (theta < 0.0f)
      theta = -theta;
    else if (theta > M_PIf32) {
      theta = 2.0f * M_PIf32 - theta;
    }
    phi += M_PIf32;
    sl::setCameraUpDir(-sl::getCameraUpDir());
    cameraIsUpright = sl::getCameraUpDir().z > 0.0;
  }
  else {
    theta = std::clamp(theta, EPS, M_PIf32 - EPS);
  }
  
  glm::vec3 pos = {
    r * sinf(theta) * cosf(phi),
    r * sinf(theta) * sinf(phi),
    r * cosf(theta)
  };
  sl::setCameraView(pos, -pos);
  sl::updateCameraView();
  update();

  if (thetaOverflow)
    cameraUpdatePin(cameraPin + mouseVector);
}
void RenderWidget::cameraUpdatePin(QPoint pos) {
  cameraPin = pos;
  cameraPinPos = sl::getCameraPos();
}

// Events
void RenderWidget::initializeGL() {
  makeCurrent();

  sl::init();
  
  // Set background
  QImage bgTex = QImage(":/textures/bg2.jpg");
  std::vector<char> bgTexData(bgTex.bits(), bgTex.bits() + bgTex.sizeInBytes());
  sl::setBackgroundTex((uint)bgTex.width(), (uint)bgTex.height(), &bgTexData);

  // Generate cluster
  uint nParticles = 200000;
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

  // Set Timer
  timer = new QTimer();
  timer->setInterval(33);
  connect(timer, &QTimer::timeout, this, &RenderWidget::timeTick);
  t0 = std::chrono::high_resolution_clock::now();
  timer->start();

  emit glInitialized();
}
void RenderWidget::resizeGL(int w, int h) {
  sl::setCameraAspect(float(w) / float(h));
  sl::updateCamera();
}
void RenderWidget::paintGL() {
  mutex.lock();
  sl::renderClassic();
  mutex.unlock();
}
void RenderWidget::timeTick() {
  std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
  float globalTime = 0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
  
  mutex.lock();
  sl::translateGlobalTime(globalTime);
  mutex.unlock();
  update();
}
