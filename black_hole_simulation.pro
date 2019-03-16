TEMPLATE = app

QT += core
QT += gui
QT += widgets

CONFIG += debug
unix:CONFIG += x11
DEFINES += _DEBUG

DESTDIR = ./bin
UI_DIR = ./tmp
MOC_DIR = ./tmp
RCC_DIR = ./tmp
OBJECTS_DIR = ./tmp

unix:TARGET = black_hole_simulation

unix:QMAKE_CXX = g++
unix:QMAKE_LINKER = g++
QMAKE_CXXFLAGS += -std=c++17

HEADERS += inc/res.hpp \
           inc/tmeas.hpp \
           inc/simulation_gl_320_es.hpp \
           inc/gui/mainwindow.hpp \
           inc/gui/renderwidget.hpp \
           inc/glinc/glinc.hpp

SOURCES += src/main.cpp \
           src/simulation_gl_320_es.cpp \
           src/tmeas.cpp \
           src/gui/mainwindow.cpp \
           src/gui/renderwidget.cpp \
           src/glinc/glinc.cpp

FORMS += gui/mainwindow.ui

RESOURCES += res/mainwindow.qrc

INCLUDEPATH += ./inc ./inc/gui ./inc/glinc ./tmp
LIBS += -lGLEW -lGL
