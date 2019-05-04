#version 320 es

precision mediump float;

uniform sampler2D diffuse;

in vec2 texCoord;

out vec4 color;

void main() {
  color = texture(diffuse, texCoord);
}
