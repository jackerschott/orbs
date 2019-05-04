#version 320 es

precision mediump float;

in vec2 pos;

out vec2 texCoord;

void main() {
  gl_Position = vec4(pos, 0.0, 1.0);
  texCoord = 0.5 * (pos + 1.0);
}
