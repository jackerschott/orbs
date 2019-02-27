#include <QtWidgets/qcolordialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qstyle.h>

#include "mainwindow.hpp"

#include <iostream>
#include <thread>

QColor clusterColor = QColor(255, 128, 0, 26);
QPixmap* clusterColorFill;

// Initialize
mainWindow::mainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::winMain()) {
  ui->setupUi(this);
  setWindowState(Qt::WindowState::WindowMaximized);

  int w = ui->lbEcColorDisplay->width();
  int h = ui->lbEcColorDisplay->height();
  clusterColorFill = new QPixmap(w, h);
  clusterColorFill->fill(clusterColor);
  ui->lbEcColorDisplay->setPixmap(*clusterColorFill);

  //std::string styleSheetSrc;
  //loadFile(INSTALL_PATH "res/mainwindow.qss", &styleSheetSrc);
  //setStyleSheet(QString(styleSheetSrc.c_str()));

  QPixmap thumbnail1 = QPixmap(":/textures/star_space_map_e.jpg");
  ui->lbBgPreset1->setPixmap(thumbnail1.scaled(ui->lbBgPreset1->width(), ui->lbBgPreset1->height(), Qt::KeepAspectRatioByExpanding));

  ui->wBgFromFile->setEnabled(false);

  connect(ui->wglRender, &renderWidget::initialized, this, &mainWindow::onWglRenderInitialized);
  connect(ui->ledRS, &QLineEdit::textEdited, this, &mainWindow::onLedRSTextEdited);
  connect(ui->rbBgFromPreset, &QRadioButton::toggled, this, &mainWindow::onRbBgFromPresetToggled);
  connect(ui->rbBgFromFile, &QRadioButton::toggled, this, &mainWindow::onRbBgFromFileToggled);
  connect(ui->pbEcChooseColor, &QPushButton::clicked, this, &mainWindow::onPbEcChooseColorClicked);
  connect(ui->pbGenCluster, &QPushButton::clicked, this, &mainWindow::onPbGenClusterClicked);
}
mainWindow::~mainWindow() {
  disconnect(ui->pbGenCluster, &QPushButton::clicked, this, &mainWindow::onPbGenClusterClicked);
  disconnect(ui->rbBgFromFile, &QRadioButton::toggled, this, &mainWindow::onRbBgFromFileToggled);
  disconnect(ui->rbBgFromPreset, &QRadioButton::toggled, this, &mainWindow::onRbBgFromPresetToggled);
  disconnect(ui->ledRS, &QLineEdit::textEdited, this, &mainWindow::onLedRSTextEdited);
  disconnect(ui->wglRender, &renderWidget::initialized, this, &mainWindow::onWglRenderInitialized);

  delete clusterColorFill;
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

// World Property Slots
void mainWindow::onLedRSTextEdited(UNUSED const QString& text) {

}
void mainWindow::onRbBgFromPresetToggled(bool checked) {
  ui->lbBgPreset1->setEnabled(checked);
}
void mainWindow::onRbBgFromFileToggled(bool checked) {
  ui->wBgFromFile->setEnabled(checked);
}

// Cluster Property Slots
void mainWindow::onPbEcChooseColorClicked() {
  clusterColor = QColorDialog::getColor(clusterColor, this, this->windowTitle(),
    QColorDialog::ColorDialogOption::ShowAlphaChannel);
  int w = ui->lbEcColorDisplay->width();
  int h = ui->lbEcColorDisplay->height();
  clusterColorFill = new QPixmap(w, h);
  clusterColorFill->fill(clusterColor);
  ui->lbEcColorDisplay->setPixmap(*clusterColorFill);
}
void mainWindow::onPbGenClusterClicked() {
  bool success;
  QMessageBox errMsgBox(QMessageBox::Icon::Warning, this->windowTitle(), "Error while parsing.", QMessageBox::Ok);

  uint nParticles = ui->ledEcNumPt->text().toInt(&success); if (!success) { errMsgBox.exec(); return; };
  float a = ui->ledEcA->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float b = ui->ledEcB->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float nx = ui->ledEcNormVecX->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float ny = ui->ledEcNormVecY->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float nz = ui->ledEcNormVecZ->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float dr = ui->ledEcDR->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };
  float dz = ui->ledEcDZ->text().toFloat(&success); if (!success) { errMsgBox.exec(); return; };

  if (a < b) {
    errMsgBox.exec();
    return;
  }

  glm::vec3 n = { nx, ny, nz };
  color palette[] = {
    { clusterColor.red() / float(CHAR_MAX),
    clusterColor.green() / float(CHAR_MAX),
    clusterColor.blue() / float(CHAR_MAX),
    clusterColor.alpha() / float(CHAR_MAX) }
  };
  float blurSizes[] = {
    1.00f
  };

  ui->wglRender->makeCurrent();
  sl::createEllipticCluster(nParticles, a, b, n, dr, dz, 1, palette, blurSizes);
}
