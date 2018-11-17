#include <iostream>
#include <thread>
#include <QtWidgets/qmessagebox.h>

#include "fileman.hpp"
#include "gui/mainwindow.hpp"

mainWindow::mainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::winMain()) {
  ui->setupUi(this);

  //std::string styleSheetSrc;
  //loadFile(INSTALL_PATH "res/mainwindow.qss", &styleSheetSrc);
  //setStyleSheet(QString(styleSheetSrc.c_str()));

  QPixmap thumbnail1 = QPixmap(":/textures/bgtex1");
  ui->lbBgPreset1->setPixmap(thumbnail1.scaled(ui->lbBgPreset1->width(), ui->lbBgPreset1->height(), Qt::KeepAspectRatioByExpanding));

  ui->wBgFromFile->setEnabled(false);

  connect(ui->wglRender, &renderWidget::initialized, this, &mainWindow::onWglRenderInitialized);
  connect(ui->ledRS, &QLineEdit::textEdited, this, &mainWindow::onLedRSTextEdited);
  connect(ui->rbBgFromPreset, &QRadioButton::toggled, this, &mainWindow::onRbBgFromPresetToggled);
  connect(ui->rbBgFromFile, &QRadioButton::toggled, this, &mainWindow::onRbBgFromFileToggled);
  connect(ui->pbGenCluster, &QPushButton::clicked, this, &mainWindow::onPbGenClusterClicked);
}

mainWindow::~mainWindow() {
  disconnect(ui->pbGenCluster, &QPushButton::clicked, this, &mainWindow::onPbGenClusterClicked);
  disconnect(ui->rbBgFromFile, &QRadioButton::toggled, this, &mainWindow::onRbBgFromFileToggled);
  disconnect(ui->rbBgFromPreset, &QRadioButton::toggled, this, &mainWindow::onRbBgFromPresetToggled);
  disconnect(ui->ledRS, &QLineEdit::textEdited, this, &mainWindow::onLedRSTextEdited);
  disconnect(ui->wglRender, &renderWidget::initialized, this, &mainWindow::onWglRenderInitialized);

  delete ui;
}

void mainWindow::onWglRenderInitialized() {
  uint nParticles = 1000000;
  float a = 10.0f;
  float b = 7.0f;
  float nx = 1.0f;
  float ny = -1.0f;
  float nz = 1.0f;
  float dr = 1.0f;
  float dz = 0.5f;

  glm::vec3 n = { nx, ny, nz };
  color palette[] = {
    { 1.00f, 0.50f, 0.00f, 0.10f }
  };
  float blurSizes[] = {
    1.00f
  };

  ui->wglRender->makeCurrent();
  sl::createEllipticCluster(nParticles, a, b, n, dr, dz, 1, palette, blurSizes);
}

void mainWindow::onLedRSTextEdited(const QString& text) {

}

void mainWindow::onRbBgFromPresetToggled(bool checked) {
  ui->lbBgPreset1->setEnabled(checked);
}

void mainWindow::onRbBgFromFileToggled(bool checked) {
  ui->wBgFromFile->setEnabled(checked);
}

void mainWindow::onPbGenClusterClicked() {
  bool success;
  QMessageBox errMsgBox(QMessageBox::Icon::Warning, this->windowTitle(), "Error while parsing.", QMessageBox::Ok);

  uint nParticles = ui->ledRnNumPt->text().toInt(&success); if (!success) { errMsgBox.exec(); return; };
  float a = ui->ledRnA->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float b = ui->ledRnB->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float nx = ui->ledRnNormVecX->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float ny = ui->ledRnNormVecY->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float nz = ui->ledRnNormVecZ->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float dr = ui->ledRnDR->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float dz = ui->ledRnDZ->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };

  if (a < b) {
    errMsgBox.exec();
    return;
  }

  glm::vec3 n = { nx, ny, nz };
  color palette[] = {
    { 1.00f, 0.50f, 0.00f, 0.10f }
  };
  float blurSizes[] = {
    1.00f
  };

  ui->wglRender->makeCurrent();
  sl::createEllipticCluster(nParticles, a, b, n, dr, dz, 1, palette, blurSizes);
}
