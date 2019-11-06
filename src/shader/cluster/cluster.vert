#version 320 es

#include "math.glsl"
#include "complex.glsl"
#include "elliptic.glsl"
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

vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
  float r1 = length(c.xyz);
  float r2 = length(p.xyz);
  if (r1 < 1.5 || r2 < 1.5) {
    outOfRange = 1;
    return;
  }
  float u1 = 1.0 / r1;
  float u2 = 1.0 / r2;
  float phi = acos(dot(c.xyz, p.xyz) * u1 * u2);
  if (invert)
    phi = 2.0 * PI - phi;
  phi += 2.0 * PI * float(nLoops);

  float b = impactParam(u1, u2, phi);
  vec2 v = b2v(b, u1, 0.0);
  if (invert)
    v = vec2(v.x, -v.y);
  
  vec3 e1 = normalize(c.xyz);
  vec3 e2 = normalize(p.xyz - dot(p.xyz, e1) * e1);
  vec3 look = v.x * e1 + v.y * e2;
  gl_Position = PV * vec4(c.xyz + look, 1.0);
  color_fs = vec4(color_vs.xyz, 1.0);
  outOfRange = 0;

  // gl_Position = PV * vec4(p.xyz, 1.0);
}

