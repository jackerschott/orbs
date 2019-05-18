TEMPLATE = app

QT += core
QT += gui
QT += widgets

CONFIG += debug
unix:CONFIG += x11
DEFINES += _DEBUG

DESTDIR = bin
UI_DIR = qt/ui
MOC_DIR = qt/moc
RCC_DIR = qt/rcc
OBJECTS_DIR = obj

unix:TARGET = black_hole_simulation

unix:QMAKE_CXX = g++
unix:QMAKE_LINKER = g++
QMAKE_CXXFLAGS += -std=c++17

HEADERS += inc/res.hpp \
           inc/tmeas.hpp \
           inc/simulation.hpp \
           inc/gui/mainwindow.hpp \
           inc/gui/renderwidget.hpp \
           inc/gl/gl.hpp

SOURCES += src/main.cpp \
           src/simulation.cpp \
           src/tmeas.cpp \
           src/gui/mainwindow.cpp \
           src/gui/renderwidget.cpp \
           src/gl/shader.cpp \
           src/gl/prog.cpp

FORMS += gui/mainwindow.ui

RESOURCES += res/mainwindow.qrc

INCLUDEPATH += ./inc ./inc/gui ./inc/gl ./tmp
LIBS += -lGLEW -lGL
