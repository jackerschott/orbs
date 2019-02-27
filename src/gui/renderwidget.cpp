#define _USE_MATH_DEFINES

#include "renderwidget.hpp"

#ifdef _WIN32
#include <GL/wglew.h>
#endif
#ifdef __unix__
namespace glxew {
#include <GL/glxew.h>
}
#endif

#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

#include "tmeas.hpp"

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
  cl::Platform clPlatform = cl::Platform::getDefault();
  cl::Device clDevice = cl::Device::getDefault();

  std::cout << "Device: \t" << clDevice.getInfo<CL_DEVICE_NAME>() << std::endl;
  std::cout << "Platform: \t" << clPlatform.getInfo<CL_PLATFORM_NAME>() << std::endl;

  // std::string ext = clDevice.getInfo<CL_DEVICE_EXTENSIONS>();
  // std::cout << ext << std::endl;

  // #ifdef _WIN32
  // cl_context_properties clContextProps[] = {
  //   CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
  //   CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
  //   CL_CONTEXT_PLATFORM, (cl_context_properties)clPlatform(),
  //   0
  // };
  // #endif
  // #ifdef __linux
  // cl_context_properties clContextProps[] = {
  //   CL_GL_CONTEXT_KHR, (cl_context_properties)glxew::glXGetCurrentContext(),
  //   CL_GLX_DISPLAY_KHR, (cl_context_properties)glxew::glXGetCurrentDrawable(),
  //   CL_CONTEXT_PLATFORM, (cl_context_properties)clPlatform(),
  //   0
  // };
  // #endif
  //cl::Context clContext(clDevice/*, clContextProps*/);

  float rs = 1.0f;
  //uint nRnPts = 10000;

  observer.pos = { 30.0f, 0.0f, 0.0f };
  observer.lookDir = -observer.pos;
  observer.upDir = { 0.0f, 0.0f, 1.0f };
  observer.fov = glm::radians(60.0f);
  observer.aspect = float(width()) / float(height());
  observer.zNear = 0.1f;
  observer.zFar = 100.0f;

  QImage bgTex = QImage(":/textures/star_space_map_e.jpg");

  // color palette[] = {
  //   { 1.00f, 0.50f, 0.00f, 0.10f }
  // };
  // float blurSizes[] = {
  //   1.00f
  // };

  sl::init(rs);
  sl::setBackgroundTex(bgTex.sizeInBytes(), bgTex.bits(), bgTex.width(), bgTex.height(), bgTex.pixelFormat().bitsPerPixel() / 8);
  sl::setObserverCamera(observer);
  //sl::createEllipse(nRnPts, 15.0f * rs, 10.0f * rs, { 1.0f, -1.0f, 1.0f }, 0.5f * rs, 0.5f * rs, 1, palette, blurSizes);
  
  timer = new QTimer();
  timer->setInterval(1);
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

  observer.pos = {
    30.0f * sin(M_PI_2 + sin(2.0 * M_PI * t / 30.0)) * cos(2.0 * M_PI * t / 60.0),
    30.0f * sin(M_PI_2 + sin(2.0 * M_PI * t / 30.0)) * sin(2.0 * M_PI * t / 60.0),
    30.0f * cos(M_PI_2 + sin(2.0 * M_PI * t / 30.0))
  };
  observer.lookDir = -observer.pos;

  sl::moveObserverCamera(observer.pos, observer.lookDir, observer.upDir);

  update();
}
