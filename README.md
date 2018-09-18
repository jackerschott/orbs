# black_hole_simulation

compile with
```
g++ src/main.cpp src/render.cpp -Iinc -lOpenCL -o black_hole_simulation.exe `pkg-config --cflags --libs gtk+-3.0`
```

for non development purpose you may want to add:
```
g++ src/main.cpp src/render.cpp -Iinc -lOpenCL -flinker-output=exec -O3 -static -o black_hole_simulation.exe `pkg-config --cflags --libs gtk+-3.0`
```
