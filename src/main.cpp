#define TEST_WITHOUT_QT false
#if !TEST_WITHOUT_QT

#include <QtWidgets/QApplication>
#include <iostream>
#include <fstream>
#include <thread>

#include "clWrapper/clwrap.hpp"
#include "glWrapper/glwrap.hpp"
#include "gui/mainwindow.hpp"
#include "simulation.hpp"
#include "tmeas.hpp"

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  mainWindow windowMain;
  windowMain.show();

  return app.exec();
}

#else
#include "tmain.hpp"

int main(int argc, char** argv) {
  return tmain(argc, argv);
}
#endif
