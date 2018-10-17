#include <iostream>

#include "gui/mainwindow.hpp"

mainWindow::mainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::winMain()) {
  ui->setupUi(this);

  connect(ui->ledRS, &QLineEdit::textEdited, this, &mainWindow::onLedRSTextEdited);

  connect(ui->rbBgFromPreset, &QRadioButton::toggled, this, &mainWindow::onRbBgFromPresetToggled);
  connect(ui->rbBgFromFile, &QRadioButton::toggled, this, &mainWindow::onRbBgFromFileToggled);

  ui->wBgFromFile->setEnabled(false);
}

mainWindow::~mainWindow() {
  disconnect(ui->rbBgFromPreset, &QRadioButton::toggled, this, &mainWindow::onRbBgFromPresetToggled);
  disconnect(ui->rbBgFromFile, &QRadioButton::toggled, this, &mainWindow::onRbBgFromFileToggled);

  delete ui;
}

void mainWindow::onLedRSTextEdited(const QString& text) {
  
}

void mainWindow::onRbBgFromPresetToggled(bool checked) {
  ui->lbBgPreset1->setEnabled(checked);
}

void mainWindow::onRbBgFromFileToggled(bool checked) {
  ui->wBgFromFile->setEnabled(checked);
}
