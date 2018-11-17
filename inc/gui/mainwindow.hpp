#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QtWidgets/qmainwindow.h>

#include "renderwidget.hpp"
#include "ui_mainwindow.hpp"

class mainWindow : public QMainWindow {
  Q_OBJECT
private:
  Ui::winMain* ui;

public:
  mainWindow(QWidget* parent = Q_NULLPTR);
  virtual ~mainWindow();

public slots:
  void onWglRenderInitialized();
  void onLedRSTextEdited(const QString& text);
  void onRbBgFromPresetToggled(bool checked);
  void onRbBgFromFileToggled(bool checked);
  void onPbGenClusterClicked();
};

#endif
