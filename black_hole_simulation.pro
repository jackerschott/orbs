TEMPLATE = app

unix:TARGET = black_hole_simulation
win32:TARGET = black_hole_simulation.exe

unix:LIBS += -lOpenCL -lGLEW -lGL

QMAKE_CXXFLAGS += -std=c++17 # CONFIG += c++17
CONFIG += debug

unix:CONFIG += x11
win32:CONFIG += windows
DEFINES += _DEBUG

DESTDIR=./bin
UI_DIR=./tmp
MOC_DIR=./tmp
RCC_DIR=./tmp
OBJECTS_DIR=./tmp

INCLUDEPATH += ./inc ./inc/gui ./inc/glinc ./tmp
DEFINES += CL_TARGET_OPENCL_VERSION=120

QT += core
QT += gui
QT += widgets

HEADERS += inc/res.hpp \
           inc/tmeas.hpp \
           inc/simulation_gl_320_es.hpp \
           inc/gui/mainwindow.hpp \
           inc/gui/renderwidget.hpp

SOURCES += src/main.cpp \
           src/res.cpp \
           src/simulation_gl_320_es.cpp \
           src/tmeas.cpp \
           src/gui/mainwindow.cpp \
           src/gui/renderwidget.cpp

FORMS += gui/mainwindow.ui

RESOURCES += res/mainwindow.qrc
