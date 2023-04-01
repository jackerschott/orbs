#version 320 es

#include "math/math.glsl"
#include "math/complex.glsl"
#include "math/elliptic.glsl"
#include "geodesic.glsl"

precision mediump float;


layout(location = 0) uniform mat4 PV;
layout(location = 1) uniform vec4 c;
layout(location = 2) uniform bool invert;
layout(location = 3) uniform uint nLoops;

in vec4 p;
in vec4 color_vs;

out vec4 color_fs;
out flat int outOfRange;

void main() {

}

