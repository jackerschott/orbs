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
  double phi = 0.0;
  double theta = M_PI_2;
  double r = 30.0;

  glm::vec3 pos = {
    r * sin(theta) * cos(phi),
    r * sin(theta) * sin(phi),
    r * cos(theta)
  };
  glm::vec3 upDir = { 0.0f, 0.0f, 1.0f };
  float aspect = float(width()) / float(height());

  QImage bgTex = QImage(":/textures/star_space_map_e.jpg");
  std::vector<char> bgTexData(bgTex.bits(), bgTex.bits() + bgTex.sizeInBytes());

  sl::init();
  sl::setBackgroundTex(bgTex.width(), bgTex.height(), &bgTexData);
  sl::setCamera(pos, -pos, upDir, glm::radians(60.0f), aspect, 0.1f, 100.0f);

  sl::updateBackgroundTex();
  sl::updateCamera();

  timer = new QTimer();
  timer->setInterval(10);
  connect(timer, &QTimer::timeout, this, &renderWidget::updateObjects);
  timer->start();
  
  initTimeMeas(true, 2.0);

  initTime = std::chrono::high_resolution_clock::now();

  emit initialized();
}

void renderWidget::resizeGL(int w, int h) {
  sl::setCameraAspect(float(w) / float(h));
  sl::updateCamera();
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

  double T = 60.0;
  double phi = 2.0 * M_PI * t / T;
  double theta = M_PI_2 + sin(2.0 * M_PI * 2.0 * t / T);
  double r = 30.0;
  glm::vec3 pos = {
    r * sin(theta) * cos(phi),
    r * sin(theta) * sin(phi),
    r * cos(theta)
  };

  sl::setCameraPos(pos);
  sl::setCameraLookDir(-pos);
  sl::updateCameraView();

  update();
}
