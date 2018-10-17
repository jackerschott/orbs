#ifndef RENDERWIDGET_HPP
#define RENDERWIDGET_HPP

#include <GL/glew.h>
#include <mutex>
#include <QtCore/qthread.h>
#include <QtWidgets/qopenglwidget.h>
#include <QtGui/qopengl.h>
#include <QtOpenGL/qgl.h>
#include <thread>

#include "render.hpp"

class renderWidget : public QOpenGLWidget {
  Q_OBJECT

private:
  camera observer;
  std::chrono::high_resolution_clock::time_point initTime;
  std::chrono::high_resolution_clock::time_point currTime;

public:
  renderWidget(QWidget* parent = Q_NULLPTR);
  virtual ~renderWidget();

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
};

#endif