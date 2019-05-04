#version 320 es

#include "math.glsl"
#include "complex.glsl"
#include "elliptic.glsl"
#include "geodesic.glsl"

#include "transf.glsl"

precision mediump float;

layout(location = 0) uniform vec4 cPos;
layout(location = 1) uniform mat4 PV;
layout(location = 2) uniform bool invert;
layout(location = 3) uniform uint nLoops;

in vec4 pos;
in vec4 color;

out vec4 color_;
out flat int outOfRange;

void main() {
  // Create rotation matrices to reduce the computations to the xy-plane
  // R  : Inverse of R_I
  // R_I : Rotates cPos and pos to the xy-plane
  mat4 R_I = red_xy(cPos.xyz, pos.xyz, invert);
  mat4 R = transpose(R_I);

  // Get the xy-plane coordinates
  vec2 p1 = (R_I * cPos).xy;
  vec2 p2 = (R_I * pos).xy;

  float r1 = length(p1);
  float r2 = length(p2);
  // Do not render points inside the critical radius r/rs = 3/2
  if (r1 < 1.5 || r2 < 1.5) {
    outOfRange = 1;
    return;
  }
  float u1 = 1.0 / r1;
  float u2 = 1.0 / r2;
  float phi1 = atan(p1.y, p1.x);
  float phi2 = atan(p2.y, p2.x);

  // Make sure phi2 >= phi1 while taking the number of loops into account
  if (phi1 > phi2) {
    phi2 += float(nLoops + 1u) * PI2;
  }
  else {
    phi2 += float(nLoops) * PI2;
  }
  
  // Compute the characterizing parameter of the geodesic
  float b = impactParam(u1, u2, phi1, phi2);
  vec2 v = b2v(b, u1, phi1);

  // Position the output point in front of the camera in the direction of v, rotate it back and apply the projection
  gl_Position = PV * R * vec4(p1 + v, 0.0, 1.0);
  color_ = color;
  outOfRange = 0;
}
