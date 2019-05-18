#include <QtWidgets/QApplication>

#include "mainwindow.hpp"

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  MainWindow windowMain;
  windowMain.show();

  return app.exec();
}
