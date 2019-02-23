#version 140

in vec2 texCoordF;

out vec4 color;

uniform sampler2D diffuse;

void main() {
  color = texture2D(diffuse, texCoordF);
}