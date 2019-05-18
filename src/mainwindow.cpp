#include <QtGui/QtEvents>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStyle>

#include "mainwindow.hpp"

#include <iostream>
#include <thread>

QColor clusterColor = QColor(255, 128, 0, 26);
QPixmap* clusterColorFill;

// Initialize
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::winMain()) {
  ui->setupUi(this);
  setWindowState(Qt::WindowState::WindowMaximized);
  QApplication::instance()->installEventFilter(this);
  setMouseTracking(true);

  int w = ui->lbEcColorDisplay->width();
  int h = ui->lbEcColorDisplay->height();
  clusterColorFill = new QPixmap(w, h);
  clusterColorFill->fill(clusterColor);
  ui->lbEcColorDisplay->setPixmap(*clusterColorFill);

  // std::string styleSheetSrc;
  // loadFile(INSTALL_PATH "res/mainwindow.qss", &styleSheetSrc);
  // setStyleSheet(QString(styleSheetSrc.c_str()));

  QPixmap thumbnail1 = QPixmap(":/textures/bg2.jpg");
  ui->lbBgPreset1->setPixmap(thumbnail1.scaled(ui->lbBgPreset1->width(), ui->lbBgPreset1->height(), Qt::KeepAspectRatioByExpanding));

  ui->wBgFromFile->setEnabled(false);

  connect(ui->wglRender, &RenderWidget::glInitialized, this, &MainWindow::onWglRenderGlInitialized);
  connect(ui->ledRS, &QLineEdit::textEdited, this, &MainWindow::onLedRSTextEdited);
  connect(ui->rbBgFromPreset, &QRadioButton::toggled, this, &MainWindow::onRbBgFromPresetToggled);
  connect(ui->rbBgFromFile, &QRadioButton::toggled, this, &MainWindow::onRbBgFromFileToggled);
  connect(ui->pbEcChooseColor, &QPushButton::clicked, this, &MainWindow::onPbEcChooseColorClicked);
  connect(ui->pbGenCluster, &QPushButton::clicked, this, &MainWindow::onPbGenClusterClicked);
}
MainWindow::~MainWindow() {
  disconnect(ui->pbGenCluster, &QPushButton::clicked, this, &MainWindow::onPbGenClusterClicked);
  disconnect(ui->rbBgFromFile, &QRadioButton::toggled, this, &MainWindow::onRbBgFromFileToggled);
  disconnect(ui->rbBgFromPreset, &QRadioButton::toggled, this, &MainWindow::onRbBgFromPresetToggled);
  disconnect(ui->ledRS, &QLineEdit::textEdited, this, &MainWindow::onLedRSTextEdited);
  disconnect(ui->wglRender, &RenderWidget::glInitialized, this, &MainWindow::onWglRenderGlInitialized);

  delete clusterColorFill;
  delete ui;
}
void MainWindow::onWglRenderGlInitialized() {
  
}

// World Property Slots
void MainWindow::onLedRSTextEdited(UNUSED const QString& text) {

}
void MainWindow::onRbBgFromPresetToggled(bool checked) {
  ui->lbBgPreset1->setEnabled(checked);
}
void MainWindow::onRbBgFromFileToggled(bool checked) {
  ui->wBgFromFile->setEnabled(checked);
}

// Cluster Property Slots
void MainWindow::onPbEcChooseColorClicked() {
  clusterColor = QColorDialog::getColor(clusterColor, this, this->windowTitle(),
    QColorDialog::ColorDialogOption::ShowAlphaChannel);
  int w = ui->lbEcColorDisplay->width();
  int h = ui->lbEcColorDisplay->height();
  clusterColorFill = new QPixmap(w, h);
  clusterColorFill->fill(clusterColor);
  ui->lbEcColorDisplay->setPixmap(*clusterColorFill);
}
void MainWindow::onPbGenClusterClicked() {
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
  std::vector<glm::vec4> palette = {
    { clusterColor.red() / float(CHAR_MAX),
    clusterColor.green() / float(CHAR_MAX),
    clusterColor.blue() / float(CHAR_MAX),
    clusterColor.alpha() / float(CHAR_MAX) }
  };
  std::vector<float> blurSizes = {
    1.00f
  };

  ui->wglRender->makeCurrent();
  sl::createEllipticCluster(nParticles, a, b, n, dr, dz, palette, blurSizes);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
  if (!ui->wglRender->isCameraMoving())
    return;

  QPoint pos = ui->wglRender->mapFromGlobal(event->globalPos());
  QRect viewRect = ui->wglRender->geometry();
  viewRect.moveTopLeft(QPoint());
  if (!ui->wglRender->geometry().contains(pos)) {
    #define MOD(a, b) (((a) < 0) ? (((a) % (b)) + (b)) : ((a) % (b)))
    pos.setX(MOD(pos.x(), viewRect.width()));
    pos.setY(MOD(pos.y(), viewRect.height()));
    cursor().setPos(ui->wglRender->mapToGlobal(pos));
    ui->wglRender->cameraUpdatePin(pos);
  }
  ui->wglRender->cameraMove(pos - ui->wglRender->getCameraPin());
}
void MainWindow::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_F11) {
    
  }
}

bool MainWindow::eventFilter(UNUSED QObject* object, QEvent* event) {
  if (event->type() == QEvent::MouseMove) {
    mouseMoveEvent(static_cast<QMouseEvent*>(event));
  }
  return false;
}
