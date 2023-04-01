#version 320 es

precision mediump float;

in vec4 color_fs;
in flat int outOfRange;

out vec4 color_out;

void main() {
	if (outOfRange != 0)
		discard;
	color_out = color_fs;
}
