#version 120
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_explicit_uniform_location : enable

in vec4 fColor;

out vec4 color;

void main() {
	color = fColor;
}
