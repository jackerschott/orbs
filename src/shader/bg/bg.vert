#version 320 es

precision mediump float;

in vec2 p;

out vec2 px;

void main() {
  gl_Position = vec4(p, 0.0, 1.0);
  px = p;
}
