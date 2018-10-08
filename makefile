all:
	g++ src/main.cpp src/render.cpp -Iinc -lOpenCL -o black_hole_simulation.exe `pkg-config --cflags --libs gtk+-3.0`
