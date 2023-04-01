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
struct CelestialSphere {
	int imgwidth;
	int imgheight;
	unsigned char* imgdata;
};

void orbs_init();
void orbs_close();
int orbs_get_last_error();

void orbs_set_observer(struct Observer* obs);
void orbs_get_observer(struct Observer* obs);
void orbs_update_observer();
void orbs_update_observer_vectors();

void orbs_set_celestial_sphere(struct CelestialSphere* firm);
void orbs_get_celestial_sphere(struct CelestialSphere* firm);
void orbs_update_celestial_sphere();
void orbs_update_celesital_sphere_imgdata();

void orbs_create_elliptic_cluster(int nParticles,
		float a, float b, vec3 n,
		float dr, float dz,
		int nBlurs, vec4* palette, float* blurSizes);
void orbs_remove_clusters();

void orbs_render();
void orbs_update_global_time(float t);

#endif /* ORBS_H */
