#version 320 es

$include "math/math.glsl"
$include "math/complex.glsl"
$include "math/elliptic.glsl"
$include "geodesic.glsl"

precision mediump float;

layout(location = 0) uniform mat4 PV;
layout(location = 1) uniform vec4 observer_pos;
layout(location = 2) uniform bool invert;
layout(location = 3) uniform uint n_loops;
layout(location = 4) uniform int geodesic_iterations;

in vec4 p;
in vec4 color_vs;

out vec4 color_fs;
out flat int out_of_range;

//#define GEODESIC_ITERATIONS geodesic_iterations

vec3 hsv2rgb(vec3 c) {
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
	float r1 = length(observer_pos.xyz);
	float r2 = length(p.xyz);
	if (r1 < 1.5 || r2 < 1.5) {
		out_of_range = 1;
		return;
	}
	float u1 = 1.0 / r1;
	float u2 = 1.0 / r2;
	float phi = acos(dot(observer_pos.xyz, p.xyz) * u1 * u2);
	if (invert)
		phi = 2.0 * PI - phi;
	phi += 2.0 * PI * float(n_loops);

	float b = impactParam(u1, u2, phi);
	vec2 v = b2v(b, u1, 0.0);
	if (invert)
		v = vec2(v.x, -v.y);
	
	/*gl_Position = vec4(r2 / 100.0 * vec2(cos(phi), sin(phi)), 0.0, 1.0);
	color_fs = vec4(hsv2rgb(vec3(phi / (2.0 * PI), 1.0, 1.0)), 1.0);*/

	vec3 e1 = normalize(observer_pos.xyz);
	vec3 e2 = normalize(p.xyz - dot(p.xyz, e1) * e1);
	vec3 look = v.x * e1 + v.y * e2;
	gl_Position = PV * vec4(observer_pos.xyz + look, 1.0);
	color_fs = color_vs;
	out_of_range = 0;

	/*gl_Position = PV * vec4(p.xyz, 1.0);
	color_fs = vec4(color_vs.xyz, 1.0);
	out_of_range = 0;*/
}

