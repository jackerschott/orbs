#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QtWidgets/QMainWindow>

#include "renderwidget.hpp"
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget* parent = Q_NULLPTR);
  virtual ~MainWindow();

public slots:
  void onWglRenderGlInitialized();
  void onLedRSTextEdited(const QString& text);
  void onRbBgFromPresetToggled(bool checked);
  void onRbBgFromFileToggled(bool checked);
  void onPbEcChooseColorClicked();
  void onPbGenClusterClicked();

protected:
  // void mousePressEvent(QMouseEvent* event) override;
  // void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

  bool eventFilter(QObject* object, QEvent* event) override;

private:
  Ui::winMain* ui;
};

#endif
