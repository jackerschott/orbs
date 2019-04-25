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

out vec4 fColor;

vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

mat4 rot(vec3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;
    
  return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
              oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
              oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
              0.0,                                0.0,                                0.0,                                1.0);
}

void main() {
  vec3 n = normalize(cross(cPos.xyz, pos.xyz));

  // float c = n.z;
  // float s = sqrt(1.0 - n.z * n.z);
  // vec2 tmp = (1.0 - c) * n.xy;

  mat4 T = rot(vec3(n.y, -n.x, 0.0), acos(n.z));
  mat4 TI = transpose(T);
  // T[0][0] = c + tmp[1] * n[1];
  // T[0][1] = -tmp[1] * n[0];
  // T[0][2] = -n[0] * s;
  // T[0][3] = 0.0;

  // T[1][0] = -tmp[0] * n[1];
  // T[1][1] = c + tmp[0] * n[0];
  // T[1][2] = s * n[1];
  // T[1][3] = 0.0;

  // T[2][0] = -s * n[0];
  // T[2][1] = -s * n[1];
  // T[2][2] = c;
  // T[2][3] = 0.0;

  // T[3][0] = 0.0;
  // T[3][1] = 0.0;
  // T[3][2] = 0.0;
  // T[3][3] = 1.0;
  
  // TI = transpose(T);

  vec2 p1 = (TI * cPos).xy;
  vec2 p2 = (TI * pos).xy;

  float phi1 = atan(p1.y, p1.x);
  float phi2 = atan(p2.y, p2.x);
  float u1 = 1.0 / sqrt(p1.x * p1.x + p1.y * p1.y);
  float u2 = 1.0 / sqrt(p2.x * p2.x + p2.y * p2.y);

  if (u2 > 2.0 / 3.0) {
    return;
  }

  if (phi1 > phi2) {
    phi2 += PI2;
  }

  float u1_sqr = u1 * u1;
  float b = impactParam(u1, u2, phi1, phi2);
  float ud1 = sqrt(1.0 / (b * b) + u1_sqr * (u1 - 1.0));
  float s_phi1 = sin(phi1);
  float c_phi1 = cos(phi1);
  vec2 v = vec2(-u1 * s_phi1 - ud1 * c_phi1, u1 * c_phi1 - ud1 * s_phi1) / u1_sqr;

  vec4 p = T * vec4(p1 + normalize(v), 0.0, 1.0);
  gl_Position = viewPj * p;
  fColor = color;

  // vec4 p_dev = viewPj * pos;
  // gl_Position = vec4(atan(p_dev.y, p_dev.x) / PI2, b / 100.0, 0.0, 1.0);
  // fColor = vec4(hsv2rgb(vec3(atan(p_dev.y, p_dev.x) / PI2, 1.0, 1.0)), color.w);
}
