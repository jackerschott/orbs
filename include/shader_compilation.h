#ifndef SHADER_COMPILATION_H
#define SHADER_COMPILATION_H

#include <GL/gl.h>

#include "shadersrc.h"

#define N_PROGS 7
#define MAX_SHADERS_PER_PROG 8

/* Access opengl programs through union to automate compilation, while
   maintaining the possiblity to address each program individually */
union gl_programs {
	struct {
		GLuint render_cluster;
		GLuint evolve_cluster;
		GLuint gen_elliptic_cluster_pos;
		GLuint gen_elliptic_cluster_colors;
		GLuint render_celestial_sphere;
		GLuint gen_uniform_samples;
		GLuint gen_gaussian_samples;
	};
	GLuint wrap[N_PROGS];
};

int compile_gl_programs(union gl_programs* progs);
void free_gl_programs(union gl_programs* progs);

#endif /* SHADER_COMPILATION_H */
