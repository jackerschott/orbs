#version 320 es

precision mediump float;

in vec4 color_;
in flat int outOfRange;

out vec4 color;

void main() {
	if (outOfRange != 0)
		discard;
	color = color_;
}
