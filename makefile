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

INCFLAGS := -Iinc
VARFLAGS := -DCL_TARGET_OPENCL_VERSION=220
LIBFLAGS := -lOpenCL -lSDL2 -lGLEW -lGL
PKGFLAGS := `pkg-config --cflags --libs gtk+-3.0`

CC := g++
CFLAGS := $(OSFLAGS) $(INCFLAGS) $(VARFLAGS) $(LIBFLAGS) $(PKGFLAGS)

CPP := src/fileman.cpp src/main.cpp src/objects3d.cpp src/render.cpp src/res.cpp src/rng.cpp src/tmain.cpp src/clWrapper/clwrap.cpp src/glWrapper/glwrap.cpp src/glWrapper/mesh.cpp src/glWrapper/shader.cpp
HPP := inc/fileman.hpp inc/objects3d.hpp inc/randutils.hpp inc/render.hpp inc/res.hpp inc/rng.hpp inc/tmain.hpp inc/clWrapper/clwrap.hpp inc/glWrapper/glwrap.hpp inc/kernels/objects3d.h inc/kernels/render.h
 
all: $(CPP) $(HPP)
	$(CC) $(CPP) $(CFLAGS) -o $(TARGET)

O3: $(CPP) $(HPP)
	$(CC) $(CPP) $(CFLAGS) -O3 -o $(TARGET)
