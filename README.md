# Black hole simulation

## Introduction
The goal of this project is a cross platform 3D-Simulation (at least linux and windows), which shall display the physical (or rather optical) relativistic effects, which occur in the environment of a black hole. This will include light deflection, gravitational effects on particles as well as red- and blue-shift, respectively.

## Building
On linux/unix systems the project can be build with qmake. Currently the build, as well as the possible resulting binaries is, or rather are, only working on linux. The current goal is to provide 'building' on linux and on windows, whereat the binaries for an operating system shall only be build on the corresponding operating system. Maybe, a cross compiling mechanism for linux to compile for windows will be added as well.

## Current project state
Because of 'some difficulties' at finding the right cross-platform libraries and way to generate and render huge particle clusters on the gpu, the actual goal of the project is not very close yet. In the current state the program is 'only' capable of generating and rendering an elliptic particle cluster (classically, as there will also be relativistic rendering methods).
The current GUI is only improvisational and can only display particle clusters and generate them with previously specified values. There is no way to let the user control the camera yet, so it is only moving a more or less random path to get a better feeling for the ouput.
But aside that, i think, all the first sticking points were overcome and, at least, all the fundamentals of the projects are set, so that one now can start with the interesting and physical parts of this projects.