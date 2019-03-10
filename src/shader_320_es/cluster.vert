#version 320 es

#define PI 3.14159265358979323846

precision mediump float;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;

layout(location = 2) uniform mat4 viewPj;

out vec4 fColor;

void main() {
  gl_Position = viewPj * pos;
  fColor = color;
}