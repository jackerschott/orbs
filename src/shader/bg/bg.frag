#version 320 es

precision mediump float;

#include "math.glsl"
#include "complex.glsl"
#include "elliptic.glsl"
#include "geodesic.glsl"

layout(location = 0) uniform vec4 c;
layout(location = 1) uniform mat2 P_I;
layout(location = 2) uniform mat4 V_I;
uniform sampler2D bgTex;

in vec2 px;

out vec4 color_out;

void main() {
  vec3 l = (V_I * vec4(P_I * px, -1.0, 1.0)).xyz;
  vec3 p = c.xyz + l;

  vec3 e1 = normalize(c.xyz);
  vec3 e2 = normalize(p - dot(p, e1) * e1);

  vec2 v = vec2(dot(e1, l), dot(e2, l));
  float u = 1.0 / length(c.xyz);
  float b = v2b(v, u, 0.0);
  if (b <= bc) {
    color_out = vec4(0.0, 0.0, 0.0, 1.0);
    return;
  }

  vec2 R1, R2, R3;
  roots(b, R1, R2, R3);
  float phi_esc = phi_plus_ext(0.0, b, u, R1, R2, R3).x;

  vec3 esc = cos(phi_esc) * e1 + sin(phi_esc) * e2;
  float phi = atan(esc.y, esc.x);
  float theta = acos(esc.z);

  vec2 px_ = vec2(0.5 + phi / PI2, theta / PI);
  color_out = texture(bgTex, px_);
}
