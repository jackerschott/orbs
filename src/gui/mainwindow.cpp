#include "gui/mainwindow.hpp"

mainWindow::mainWindow(QWidget* parent) : QMainWindow(parent) {  
  ui.setupUi(this);

  connect(ui.rbBgFromPreset, &QRadioButton::toggled, this, &mainWindow::onRbBgFromPresetToggled);
  connect(ui.rbBgFromFile, &QRadioButton::toggled, this, &mainWindow::onRbBgFromFileToggled);

  ui.wBgFromFile->setEnabled(false);
}

mainWindow::~mainWindow() {

}

void mainWindow::onRbBgFromPresetToggled(bool checked) {
  ui.lbBgPreset1->setEnabled(checked);
}

void mainWindow::onRbBgFromFileToggled(bool checked) {
  ui.wBgFromFile->setEnabled(checked);
}
