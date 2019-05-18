#ifndef RENDERWIDGET_HPP
#define RENDERWIDGET_HPP

#include <GL/glew.h>

#include <QtGui/QtEvents>
#include <QtCore/QTimer>
#include <QtWidgets/QOpenGLWidget> 

#include "simulation.hpp"

class RenderWidget : public QOpenGLWidget {
  Q_OBJECT

public:
  RenderWidget(QWidget* parent = Q_NULLPTR);
  virtual ~RenderWidget();

  bool isCameraMoving();
  QPoint getCameraPin();

signals:
  void glInitialized();

public:
  void cameraMove(QPoint mouseVector);
  void cameraUpdatePin(QPoint pos);

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

private:
  QSurfaceFormat format;

  const float dotsPerTurn = 2000.0f;
  const float facPerTurn = 10.0f;
  bool cameraIsMoving;
  QPoint cameraPin;
  glm::vec3 cameraPinPos;
  bool cameraIsUpright = true;
};

#endif
