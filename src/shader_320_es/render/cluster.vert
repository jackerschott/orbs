#version 320 es

#include "float.glsl"
#include "complex.glsl"
#include "elliptic.glsl"
#include "geodesic.glsl"

precision mediump float;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;

layout(location = 3) uniform vec4 cPos;
layout(location = 4) uniform mat4 viewPj;
layout(location = 5) uniform bool invert;
layout(location = 6) uniform uint nLoops;

out vec4 fColor;

mat4 rot(vec3 axis, float angle);

void main() {
  // Create rotation matrices to reduce the computations to the xy-plane
  // R  : Rotates cPos and pos to the xy-plane
  // RI : Inverse of R
  vec3 n = normalize(cross(cPos.xyz, pos.xyz));
  if (invert)
    n = -n;
  mat4 R = rot(vec3(n.y, -n.x, 0.0), acos(n.z));
  mat4 RI = transpose(R);

  // Get the xy-plane coordinates
  vec2 p1 = (R * cPos).xy;
  vec2 p2 = (R * pos).xy;
  float phi1 = atan(p1.y, p1.x);
  float phi2 = atan(p2.y, p2.x);
  float u1 = 1.0 / sqrt(p1.x * p1.x + p1.y * p1.y);
  float u2 = 1.0 / sqrt(p2.x * p2.x + p2.y * p2.y);

  // Do not render points inside the critical radius r/rs = 3/2
  if (u2 > 2.0 / 3.0) {
    return;
  }

  // Make sure phi2 >= phi1 while taking the number of loops into account
  if (phi1 > phi2) {
    phi2 += float(nLoops + 1u) * PI2;
  }
  else {
    phi2 += float(nLoops) * PI2;
  }
  
  // TODO: Remove
  // Ignore problematic points which are right in front of the camera
  if (phi2 - phi1 > -0.07 && phi2 - phi1 < 0.07) {
    return;
  }
  
  // Compute the characterizing parameter of the geodesic
  float b = impactParam(u1, u2, phi1, phi2);
  vec2 v = initVector(b, u1, phi1);

  // Position the output point in front of the camera in the direction of v, rotate it back and apply the projection
  gl_Position = viewPj * RI * vec4(p1 + v, 0.0, 1.0);
  fColor = color;
}

mat4 rot(vec3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;
  
  return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y + axis.z * s,  oc * axis.z * axis.x - axis.y * s,  0.0,
              oc * axis.x * axis.y - axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z + axis.x * s,  0.0,
              oc * axis.z * axis.x + axis.y * s,  oc * axis.y * axis.z - axis.x * s,  oc * axis.z * axis.z + c,           0.0,
              0.0,                                0.0,                                0.0,                                1.0);
}
