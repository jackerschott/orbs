#define _USE_MATH_DEFINES

#include <cmath>
#include <iostream>

#include "gui/renderwidget.hpp"
#include "tmeas.hpp"

#ifdef _WIN32
#include <GL/wglew.h>
#endif
#ifdef __unix__
#include <GL/glxew.h>
#endif

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
  
  render::close();
}

void renderWidget::initializeGL() {
  cl::Platform clPlatform = cl::Platform::getDefault();
  cl::Device clDevice = cl::Device::getDefault();
  cl_context_properties clContextProps[] = {
    CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
    CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
    CL_CONTEXT_PLATFORM, (cl_context_properties)clPlatform(),
    0
  };
  cl::Context clContext(clDevice, clContextProps);

  float rs = 1.0f;
  uint nRnPts = 5000;

  observer.pos = { 30.0f, 0.0f, 0.0f };
  observer.lookDir = -observer.pos;
  observer.upDir = { 0.0f, 0.0f, 1.0f };
  observer.fov = glm::radians(60.0f);
  observer.aspect = float(width()) / float(height());
  observer.zNear = 0.1f;
  observer.zFar = 100.0f;

  QImage bgTex = QImage(":/img/bgtex1");

  colorBlur palette[] = {
    { { 1.00f, 0.30f, 0.00f, 0.40f }, 0.990f },
    { { 0.25f, 0.60f, 1.00f, 1.00f }, 0.005f },
    { { 1.00f, 1.00f, 1.00f, 1.00f }, 0.005f }
  };

  render::init(rs, clDevice, clContext);
  render::setBackgroundTex(bgTex.sizeInBytes(), bgTex.bits(), bgTex.width(), bgTex.height(), bgTex.pixelFormat().bitsPerPixel() / 8);
  render::setObserverCamera(observer);
  render::createParticleRing(nRnPts, 10.0f * rs, { 1.0f, -1.0f, 1.0f }, 1.0f * rs, 0.1f, 0.1f, 3, palette);
  
  timer = new QTimer();
  timer->setInterval(std::chrono::milliseconds(1));
  connect(timer, &QTimer::timeout, this, &renderWidget::updateObjects);
  timer->start();
  
  initTimeMeas(true, 2.0);

  initTime = std::chrono::high_resolution_clock::now();
}

void renderWidget::resizeGL(int w, int h) {
  render::setObserverCameraAspect(float(w) / float(h));
}

void renderWidget::paintGL() {
  setTimeMeasPoint();
  render::renderClassic();
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

  observer.pos = {
    30.0f * sin(M_PI_2 + sin(2.0 * M_PI * t / 30.0)) * cos(2.0 * M_PI * t / 60.0),
    30.0f * sin(M_PI_2 + sin(2.0 * M_PI * t / 30.0)) * sin(2.0 * M_PI * t / 60.0),
    30.0f * cos(M_PI_2 + sin(2.0 * M_PI * t / 30.0))
  };
  observer.lookDir = -observer.pos;

  render::moveObserverCamera(observer.pos, observer.lookDir, observer.upDir);

  update();
}
