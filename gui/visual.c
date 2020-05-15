#include <math.h>

#include "load_textures.h"
#include "orbs.h"
#include "visual.h"

#define PI      3.14159265358979323846
#define PI2     6.28318530717958647693
#define PI_2    1.57079632679489661923

/* variables for pointer camera movement */
vec2 pin_screen_point;
struct Observer observer;
struct Observer pin_observer;
struct Firmament firmament;

/* Get axis and angle for a rotation around the origin, characterized by a shift
 * vector, which specifies horizontal and vertical rotation (in units of 360
 * degree rotations).
 * look_dir and up_dir are assumed to be normalized.
 */
void center_rotation_params(vec3 pos, vec3 look_dir, vec3 up_dir,
		vec2 screen_shift, float* angle, vec3 axis)
{
	vec3 s1, s2;
	glm_vec3_cross(look_dir, up_dir, s1);
	glm_vec3_copy(up_dir, s2);

	/* account for other direction of y-axis on a screen */
	glm_vec3_scale(s1, screen_shift[0], s1);
	glm_vec3_scale(s2, -screen_shift[1], s2);

	vec3 shift;
	glm_vec3_add(s1, s2, shift);

	glm_vec3_cross(look_dir, shift, axis);
	*angle = PI2 * glm_vec3_norm(shift);
}

/* Rotate camera vectors around the specified axis with the specified angle */
void center_rotate( vec3 pos, vec3 look_dir, vec3 up_dir,
		float angle, vec3 axis)
{
	glm_vec3_rotate(pos, angle, axis);
	glm_vec3_rotate(look_dir, angle, axis);
	glm_vec3_rotate(up_dir, angle, axis);
}

void vis_setup()
{
	orbs_init();

	vec3 pos_init = { 30.0f, 0.0f, 0.0f };
	vec4 look_dir_init = { -1.0f, 0.0f, 0.0f };
	vec3 up_dir_init = { 0.0f, 0.0f, 1.0f };
	
	glm_vec3_copy(pos_init, observer.pos);
	glm_vec3_copy(look_dir_init, observer.look_dir);
	glm_vec3_copy(up_dir_init, observer.up_dir);
	observer.fov = 60.0f * PI / 180.0f;
	observer.z_near = 0.1f;
	observer.z_far = 100.0f;
	orbs_set_observer(&observer);
	orbs_update_observer();

	unsigned char *tex_data;
	unsigned int tex_width;
	unsigned int tex_height;
	load_jpeg_texture_image("Resources/bg2.jpg",
			&tex_width, &tex_height, &tex_data);

	firmament.imgdata = tex_data;
	firmament.imgwidth = tex_width;
	firmament.imgheight = tex_height;
	orbs_set_firmament(&firmament);
	orbs_update_firmament();

	unsigned int n_particles = 75000;
	float a = 20.0f;
	float b = 15.0f;
	vec3 n = { 1.0f, -1.0f, 1.0f };
	float dr = 1.0f;
	float dz = 0.5f;
	vec4 palette[] = { { 1.00f, 0.30f, 0.00f, 0.5f } };
	float blurSizes[] = { 2.00f };
	orbs_create_elliptic_cluster(n_particles,
			a, b, n, dr, dz,
			1, palette, blurSizes);
}
void vis_cleanup()
{
	orbs_close();
	free(firmament.imgdata);
}

/* Scale distance of observer from r = 1 (schwarzschild radius) with fac */
void vis_scale_observer(float fac)
{
	orbs_get_observer(&observer);
	float distance_rs = glm_vec3_norm(observer.pos) - 1.0f;
	float fac_r = (fac * distance_rs + 1.0f) / (distance_rs + 1.0f);
	glm_vec3_scale(observer.pos, fac_r, observer.pos);
	orbs_set_observer(&observer);
	orbs_update_observer_vectors();
}

/* Set starting point (pin) for pointer camera movement */
void vis_pin_observer(float x, float y)
{
	pin_screen_point[0] = x;
	pin_screen_point[1] = y;

	orbs_get_observer(&pin_observer);
}

/* Move camera projecting the line from the pin, to the current position of the
 * pointer onto a sphere with a radius equal to the distance of the camera from
 * the origin.
 */
void vis_move_pinned_observer(float x, float y)
{
	vec2 screen_shift;
	vec2 screen_point = { x, y };
	glm_vec2_sub(screen_point, pin_screen_point, screen_shift);

	glm_vec3_copy(pin_observer.pos, observer.pos);
	glm_vec3_copy(pin_observer.look_dir, observer.look_dir);
	glm_vec3_copy(pin_observer.up_dir, observer.up_dir);

	vec3 axis;
	float angle;
	center_rotation_params(observer.pos,
			observer.look_dir,
			observer.up_dir,
			screen_shift, &angle, axis);
	center_rotate(observer.pos,
			observer.look_dir,
			observer.up_dir,
			angle, axis);

	orbs_set_observer(&observer);
	orbs_update_observer_vectors();
}

void vis_set_aspect(float aspect)
{
	observer.aspect = aspect;
	orbs_set_observer(&observer);
	orbs_update_observer();
}

void vis_update()
{
	orbs_render();
}
