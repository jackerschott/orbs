#ifndef ORBS_H
#define ORBS_H

#include <cglm/cglm.h>

struct Observer {
	vec3 pos;
	vec3 look_dir;
	vec3 up_dir;
	float fov;
	float aspect;
	float z_near;
	float z_far;
};
struct Firmament {
	int imgwidth;
	int imgheight;
	unsigned char* imgdata;
};

void orbs_init();
void orbs_close();
int orbs_get_last_error();

/*
void orbs_get_camera_pos(vec3 pos);
void orbs_get_camera_look_dir(vec3 lookDir);
void orbs_get_camera_up_dir(vec3 upDir);
float orbs_get_camera_fov();
float orbs_get_camera_aspect();
float orbs_get_camera_znear();
float orbs_get_camera_zfar();
void orbs_set_camera(vec3 pos, vec3 lookDir, vec3 upDir,
		float fov, float aspect, float zNear, float zFar);
void orbs_set_camera_view(vec3 pos, vec3 lookDir);
void orbs_set_camera_pos(vec3 pos);
void orbs_set_camera_look_dir(vec3 lookDir);
void orbs_set_camera_up_dir(vec3 upDir);
void orbs_set_camera_fov(float fov);
void orbs_set_camera_aspect(float aspect);
void orbs_set_camera_znear(float zNear);
void orbs_set_camera_zfar(float zFar);
*/
void orbs_set_observer(struct Observer* obs);
void orbs_get_observer(struct Observer* obs);
void orbs_update_observer();
void orbs_update_observer_vectors();

/*void orbs_set_background_tex(unsigned int width,
		unsigned int height,
		char* data);
void orbs_set_background_tex_size(int width, int height);
void orbs_set_background_tex_data(char* data);
void orbs_update_background_tex();
void orbs_update_background_tex_data();*/
void orbs_set_firmament(struct Firmament* firm);
void orbs_get_firmament(struct Firmament* firm);
void orbs_update_firmament();
void orbs_update_firmament_imgdata();

void orbs_create_elliptic_cluster(int nParticles,
		float a, float b, vec3 n,
		float dr, float dz,
		int nBlurs, vec4* palette, float* blurSizes);
void orbs_remove_clusters();

void orbs_render();
void orbs_update_global_time(float t);

#endif /* ORBS_H */
