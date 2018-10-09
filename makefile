ifeq ($(OS),Windows_NT) 
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

ifeq ($(detected_OS),Windows)
    OSFLAGS := -D WIN32
    TARGET := black_hole_simulation.exe
endif
ifeq ($(detected_OS),Linux)
    OSFLAGS := -D LINUX -fno-pie -no-pie
    TARGET := black_hole_simulation
endif

CC := g++
INCFLAGS := -Iinc
LIBFLAGS := -lOpenCL
PKGFLAGS := `pkg-config --cflags --libs gtk+-3.0`
CFLAGS = $(OSFLAGS) $(INCFLAGS) $(LIBFLAGS) $(PKGFLAGS)

CPP := src/main.cpp src/render.cpp src/res.cpp src/rng.cpp
HPP := inc/render.hpp inc/res.hpp inc/rng.hpp inc/objects3d.hpp

all: $(CPP) $(HPP)
    $(CC) $(CPP) $(CFLAGS) -o $(TARGET)
