#version 320 es

precision mediump float;

in vec2 texCoordF;

out vec4 color;

uniform sampler2D diffuse;

void main() {
  color = texture(diffuse, texCoordF);
}