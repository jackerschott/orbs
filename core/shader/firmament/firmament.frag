#version 320 es

precision mediump float;

$include "math/math.glsl"
$include "math/complex.glsl"
$include "math/elliptic.glsl"
$include "geodesic.glsl"

layout(location = 0) uniform vec4 observer_pos;
layout(location = 1) uniform mat2 P_I;
layout(location = 2) uniform mat4 V_I;
uniform sampler2D bg_tex;

in vec2 pos;

out vec4 color_out;

void main() {
	vec4 screen_point = vec4(P_I * pos, -1.0, 1.0);
	vec3 eye_screen_vec = (V_I * screen_point).xyz;
	vec3 eye_screen_point = observer_pos.xyz + eye_screen_vec;

	vec3 e1 = normalize(observer_pos.xyz);
	vec3 e2 = normalize(eye_screen_point - dot(eye_screen_point, e1) * e1);

	vec2 light_dir = vec2(dot(e1, eye_screen_vec), dot(e2, eye_screen_vec));
	float u = 1.0 / length(observer_pos.xyz);
	float impact_param = v2b(light_dir, u, 0.0);
	if (impact_param <= bc) {
		color_out = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec2 R1, R2, R3;
	roots(impact_param, R1, R2, R3);
	float phi_esc = phi_plus_ext(0.0, impact_param, u, R1, R2, R3).x;

	vec3 esc = cos(phi_esc) * e1 + sin(phi_esc) * e2;
	
	/*vec3 esc = light_dir.x * e1 + light_dir.y * e2;*/
	
	float phi = atan(esc.y, esc.x);
	float theta = acos(esc.z);

	vec2 px_ = vec2(0.5 + phi / PI2, theta / PI);
	color_out = texture(bg_tex, px_);
}
