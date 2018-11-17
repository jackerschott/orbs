#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QtWidgets/qmainwindow.h>

#include "renderwidget.hpp"
#include "../../tmp/ui_mainwindow.h"

class mainWindow : public QMainWindow {
  Q_OBJECT
private:
  Ui::winMain* ui;

public:
  mainWindow(QWidget* parent = Q_NULLPTR);
  virtual ~mainWindow();

public slots:
  void onLedRSTextEdited(const QString& text);
  void onRbBgFromPresetToggled(bool checked);
  void onRbBgFromFileToggled(bool checked);
};

#endif
