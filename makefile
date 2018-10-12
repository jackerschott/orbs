ifeq ($(OS),Windows_NT) 
	detected_OS := Windows
else
	detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

ifeq ($(detected_OS),Windows)
	OS_FLAGS := -D WIN
	TARGET := out/debug/BlackHoleSimulation.exe
	INC_PATH_FLAGS := -I"E:/Benutzerdateien/Documents/Visual Studio 2017/Projects/BlackHoleSimulation/inc" -I$(GTK_DIR)include/gtk-3.0 -I$(GTK_DIR)include/cairo -I$(GTK_DIR)include -I$(GTK_DIR)include/pango-1.0 -I$(GTK_DIR)include/fribidi -I$(GTK_DIR)include/atk-1.0 -I$(GTK_DIR)include/cairo -I$(GTK_DIR)include/pixman-1 -I$(GTK_DIR)include -I$(GTK_DIR)include/freetype2 -I$(GTK_DIR)include -I$(GTK_DIR)include/harfbuzz -I$(GTK_DIR)include/libpng16 -I$(GTK_DIR)include/gdk-pixbuf-2.0 -I$(GTK_DIR)include/libpng16 -I$(GTK_DIR)include -I$(GTK_DIR)include/glib-2.0 -I$(GTK_DIR)lib/glib-2.0/include -I$(GTK_DIR)include -I"$(CUDA_PATH)/include" -I$(CPP_LIB_DIR)SDL/include -I$(CPP_LIB_DIR)OpenGL/glew/include -I$(CPP_LIB_DIR)OpenGL/glm
	LIB_PATH_FLAGS := -L$(CPP_LIB_DIR)SDL/lib/x64 -L$(CPP_LIB_DIR)OpenGL/glew/lib/Release/x64 -L"$(CUDA_PATH)/lib/x64"
	LIB_FLAGS := -lgtk-3 -lgdk-3 -lgdi32 -limm32 -lshell32 -lole32 -luuid -lwinmm -ldwmapi -lsetupapi -lcfgmgr32 -lz -lpangowin32-1.0 -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl -lOpenCL -lSDL2 -lSDL2main -lSDL2test -lglew32 -lglew32s -lOpenGL32

	CFLAGS := $(OSFLAGS) $(INC_PATH_FLAGS) $(LIB_PATH_FLAGS) $(LIB_FLAGS)
endif
ifeq ($(detected_OS),Linux)
	OSFLAGS := -D LINUX -fno-pie -no-pie
	TARGET := black_hole_simulation
	INCFLAGS := -I inc
	VARFLAGS := -D CL_TARGET_OPENCL_VERSION=220
	LIBFLAGS := -l OpenCL -l SDL2 -l GLEW -l OpenGL
	PKGFLAGS := `pkg-config --cflags --libs gtk+-3.0`

	CFLAGS := $(OSFLAGS) $(INCFLAGS) $(VARFLAGS) $(LIBFLAGS) $(PKGFLAGS)
endif

CC := g++

CPP := src/fileman.cpp src/main.cpp src/render.cpp src/res.cpp src/rng.cpp src/tmain.cpp src/clWrapper/clwrap.cpp src/glWrapper/glwrap.cpp
HPP := inc/fileman.hpp inc/randutils.hpp inc/render.hpp inc/res.hpp inc/rng.hpp inc/tmain.hpp inc/clWrapper/clwrap.hpp inc/glWrapper/glwrap.hpp

all: $(CPP) $(HPP)
	$(CC) $(CPP) $(CFLAGS) -o $(TARGET)

O3: $(CPP) $(HPP)
	$(CC) $(CPP) $(CFLAGS) -O3 -o $(TARGET)
