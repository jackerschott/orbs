#version 120
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec2 texCoordF;

out vec4 color;

uniform sampler2D diffuse;

void main() {
  color = texture2D(diffuse, texCoordF);
}