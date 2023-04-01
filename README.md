# orbs -- Optical Real-time Black hole Simulation

## Introduction
> This is a work in progress. Use at your own risk.

orbs will be a hardware-accelerated cross-platform simulation of black holes in
real-time.
If the latter is even possible depents on the physical effects
included in the simulation though.
The optical effects of a black hole which are simulated will be made visible
through particle clusters and a firmament rendered for a virtual observer while
respecting the relativistic effects of the black-hole on light.
The shape of these clusters will be manipulatable by the user and is filled
randomly by particles, while the firmament is rendered from a spherical texture
map.

## Features
* Rendering of particle clusters
* Available rendering effects
    * Non-retarded light-deflection
* Pointer/Mouse governed observer-control

## Future Features
* Rendering of firmament textures
* Rendering effects
    * Red/Blue-shift
    * Retarded light-deflection
* Dynamic effects
    * gravitational particle dynamics
    * gravitational observer dynamics
* Keyboard governed observer-control

## Screenshots

## Building
### Linux
orbs is build using the commands

## Current project state
At the time writing this (18/5/2019) the project is far from being fully developed. Although the GUI is only provisional just yet, some camera movement and the basic rendering code for the particles and the background is working, which means that there are no obvious visual errors, the framerate at 1000000 particles is quite good (with an NVIDIA GeForce GTX 1050 Ti) and the program does not crash if you use it normally.  
An image produced by the current project state is below

![alt text](screenshot.png)
