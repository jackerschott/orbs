#define _USE_MATH_DEFINES

#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

#include "tmeas.hpp"
#include "renderwidget.hpp"

std::vector<double> renderTime;

renderWidget::renderWidget(QWidget* parent) : QOpenGLWidget(parent) {
  format.setDepthBufferSize(32);
  format.setStencilBufferSize(8);
  format.setVersion(3, 1);
  format.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(format);
}
renderWidget::~renderWidget() {
  glMainContext = context();

  std::vector<double> meanRenderTime;
  std::vector<double> stdDevMeanRenderTime;
  evalTimeMeas(&meanRenderTime, &stdDevMeanRenderTime);
  std::cout << std::endl;
  std::cout << "Render time: " << meanRenderTime[0] << " +- "
    << stdDevMeanRenderTime[0] << " s," << "\t\t"
    << "Frame rate: " << 1.0 / meanRenderTime[0] << " +- "
    << stdDevMeanRenderTime[0] / (meanRenderTime[0] * meanRenderTime[0]) << " fps" << std::endl;
  
  delete timer;
  sl::close();
}

void renderWidget::initializeGL() {
  float rs = 1.0f;

  double phi = 0.0;
  double theta = M_PI_2;
  double r = 30.0;
  observer.pos = {
    r * sin(theta) * cos(phi),
    r * sin(theta) * sin(phi),
    r * cos(theta)
  };
  // observer.pos = { 0.0f, 0.0f, 30.0f };
  observer.lookDir = -observer.pos;
  observer.upDir = { 0.0f, 1.0f, 0.0f };
  observer.fov = glm::radians(60.0f);
  observer.aspect = float(width()) / float(height());
  observer.zNear = 0.1f;
  observer.zFar = 100.0f;

  QImage bgTex = QImage(":/textures/star_space_map_e.jpg");

  sl::init(rs);
  sl::setBackgroundTex(bgTex.sizeInBytes(), bgTex.bits(), bgTex.width(), bgTex.height(), bgTex.pixelFormat().bitsPerPixel() / 8);
  sl::setObserverCamera(observer);
  
  timer = new QTimer();
  timer->setInterval(10);
  connect(timer, &QTimer::timeout, this, &renderWidget::updateObjects);
  timer->start();
  
  initTimeMeas(true, 2.0);

  initTime = std::chrono::high_resolution_clock::now();

  emit initialized();
}

void renderWidget::resizeGL(int w, int h) {
  sl::setObserverCameraAspect(float(w) / float(h));
}
void renderWidget::paintGL() {
  setTimeMeasPoint();
  sl::renderClassic();
  setTimeMeasPoint();

  if (evalLap(&renderTime)) {
    std::cout << "Render time: " << renderTime[0] << " s," << "\t\t"
      << "Frame rate: " << 1.0 / renderTime[0] << " fps" << std::endl;
    renderTime.clear();
  }
}
void renderWidget::updateObjects() {
  currTime = std::chrono::high_resolution_clock::now();
  double t = std::chrono::duration_cast<std::chrono::nanoseconds>(currTime - initTime).count() / 1.0e9;
  // TODO: Use t

  double phi = 2.0 * M_PI * t / 60.0;
  double theta = M_PI_2 + sin(2.0 * M_PI * t / 30.0);
  double r = 30.0;
  observer.pos = {
    r * sin(theta) * cos(phi),
    r * sin(theta) * sin(phi),
    r * cos(theta)
  };
  observer.lookDir = -observer.pos;
  observer.upDir = glm::vec3(0.0, 0.0, 1.0);

  sl::moveObserverCamera(observer.pos, observer.lookDir, observer.upDir);

  update();
}
