all:
	g++ src/main.cpp src/render.cpp src/res.cpp src/rng.cpp -Iinc -lOpenCL -o black_hole_simulation.exe `pkg-config --cflags --libs gtk+-3.0`
