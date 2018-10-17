#define TEST_WITHOUT_QT false
#if !TEST_WITHOUT_QT

#include <fstream>
#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h>
#endif
#ifdef __unix__
#include <GL/glxew.h>
#endif
#include <iostream>
#include <QtWidgets/qapplication.h>
#include <thread>

#include "clWrapper/clwrap.hpp"
#include "glWrapper/glwrap.hpp"
#include "gui/mainwindow.hpp"
#include "render.hpp"
#include "tmeas.hpp"

std::thread renderThread;

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
