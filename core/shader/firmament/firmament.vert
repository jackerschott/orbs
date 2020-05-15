#version 320 es

precision mediump float;

in vec2 _pos;

out vec2 pos;

void main() {
	gl_Position = vec4(_pos, 0.0, 1.0);
	pos = _pos;
}
