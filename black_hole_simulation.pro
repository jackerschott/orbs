TEMPLATE = app

unix:TARGET = black_hole_simulation
win32:TARGET = black_hole_simulation.exe

unix:LIBS += -lOpenCL -lGLEW -lGL -lSDL2

QMAKE_CXXFLAGS += -std=c++17 # CONFIG += c++17
CONFIG += debug

unix:CONFIG += x11
win32:CONFIG += windows

DESTDIR=./bin
UI_DIR=./tmp
MOC_DIR=./tmp
RCC_DIR=./tmp
OBJECTS_DIR=./tmp

INCLUDEPATH += ./inc
DEFINES += CL_TARGET_OPENCL_VERSION=120

QT += core
QT += gui
QT += widgets

HEADERS += inc/fileman.hpp \
           inc/randutils.hpp \
           inc/res.hpp \
           inc/tmain.hpp \
           inc/tmeas.hpp \
           inc/simulation_ei.hpp \
           inc/clWrapper/clwrap.hpp \
           inc/glWrapper/glwrap.hpp \
           inc/gui/mainwindow.hpp \
           inc/gui/renderwidget.hpp \
           tmp/ui_mainwindow.h

SOURCES += src/fileman.cpp \
           src/main.cpp \
           src/res.cpp \
           src/simulation_ei.cpp \
           src/tmain.cpp \
           src/tmeas.cpp \
           src/clWrapper/clwrap.cpp \
           src/glWrapper/glwrap.cpp \
           src/gui/mainwindow.cpp \
           src/gui/renderwidget.cpp

FORMS += gui/mainwindow.ui

RESOURCES += res/mainwindow.qrc
