all:
	g++ src/main.cpp src/render.cpp src/res.cpp src/rng.cpp -Iinc -lOpenCL -fno-pie -no-pie -o black_hole_simulation `pkg-config --cflags --libs gtk+-3.0`
