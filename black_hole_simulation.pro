TEMPLATE = app

unix:TARGET = black_hole_simulation
win32:TARGET = black_hole_simulation.exe

unix:LIBS += -lOpenCL -lGLEW -lGL -lSDL2
unix:QMAKE_LFLAGS += -no-pie

DESTDIR=./bin
UI_DIR=./inc/gui
MOC_DIR=./tmp
RCC_DIR=./tmp
OBJECTS_DIR=./tmp

INCLUDEPATH += ./inc
DEFINES += CL_TARGET_OPENCL_VERSION=120

QT += core
QT += gui
QT += widgets

# Input
HEADERS += inc/fileman.hpp \
           inc/inist.hpp \
           inc/motion.hpp \
           inc/randutils.hpp \
           inc/render.hpp \
           inc/res.hpp \
           inc/rng.hpp \
           inc/tmain.hpp \
           inc/tmeas.hpp \
           inc/clWrapper/clwrap.hpp \
           inc/glWrapper/glwrap.hpp \
           inc/gui/mainwindow.hpp \
           inc/gui/renderwidget.hpp \
           inc/gui/ui_mainwindow.h
FORMS += gui/mainwindow.ui
SOURCES += src/fileman.cpp \
           src/inist.cpp \
           src/main.cpp \
           src/motion.cpp \
           src/render.cpp \
           src/res.cpp \
           src/rng.cpp \
           src/tmain.cpp \
           src/tmeas.cpp \
           src/clWrapper/clwrap.cpp \
           src/glWrapper/glwrap.cpp \
           src/gui/mainwindow.cpp \
           src/gui/renderwidget.cpp
RESOURCES += res/mainwindow.qrc
