#include <QtWidgets/QApplication>

#include "mainwindow.hpp"

#include <iostream>
#include <fstream>
#include <thread>

#include "tmeas.hpp"

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  mainWindow windowMain; 
  windowMain.show();

  return app.exec();
}
