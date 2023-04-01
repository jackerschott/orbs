#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "shadersrc.h"
#define SC_MAX_SHADERS_PER_PROG 2
#include "shader_compilation.h"
#include "orbs.h"

/* State and error definitions */
#define STATE_CLOSED	    	0
#define STATE_INIT	      	(1<<0)
#define STATE_HAS_OBSERVER 	(1<<1)
#define STATE_HAS_BG_TEXTURE    (1<<2)

#define ERR_NONE 		0
#define ERR_WRONG_STATE 	1
/* Should probably be removed in the future */
#define ERR_INTERNAL 		2

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

/* Some math constants (for some reason i did not use those in math.h
 * probably precision or the consistency because i wanted 2*PI to be defined)
 */
#define PI      3.14159265358979323846
#define PI2     6.28318530717958647693
#define PI_2    1.57079632679489661923

union gl_programs programs;

/* Particle cluster rendering */
#define CLUSTER_CAPACITY 16

struct ptcluster {
	int n_verts;
	GLuint vert_array;
	GLuint position_buf;
	GLuint color_buf;
	GLuint velocity_buf;
	GLuint angmomentum_buf;
	GLuint energy_buf;
};
int nClusters = 0;
struct ptcluster *clusters;

float globalTime;

/* Camera */
static struct Observer observer;
mat4 P; /* Projection matrix */
mat2 P_I; /* 'inverse' of P, needed for bg rendering */
mat4 V; /* View rotation matrix */

/* Background rendering */
static struct CelestialSphere celestial_sphere;

GLuint glBgVerts;
GLuint glBgPosBuf;
GLuint glBgElementBuf;
GLuint glBgTex;
GLuint glBgRenderTex;

/* Debugging and error handling*/
int state = STATE_CLOSED;
int errLast;

/* Only used for debug purposes */
const char* get_err_source(GLenum source)
{
	switch (source)
	{
	case GL_DEBUG_SOURCE_API: return "API";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window system";
	case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader compiler";
	case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third party";
	case GL_DEBUG_SOURCE_APPLICATION: return "Application";
	case GL_DEBUG_SOURCE_OTHER: return "Other";
	default: return "Unknown error source";
	}
}
const char* get_err_type(GLenum type)
{
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR: return "Error";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated behavior";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined behavior";
	case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
	case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
	case GL_DEBUG_TYPE_MARKER: return "Marker";
	case GL_DEBUG_TYPE_PUSH_GROUP: return "Push group";
	case GL_DEBUG_TYPE_POP_GROUP: return "Pop Group";
	case GL_DEBUG_TYPE_OTHER: return "Other";
	default: return "Unknown error type";
	}
}
const char* get_err_severity(GLenum severity)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH: return "Error";
	case GL_DEBUG_SEVERITY_MEDIUM: return "Major warning";
	case GL_DEBUG_SEVERITY_LOW: return "Warning";
	case GL_DEBUG_SEVERITY_NOTIFICATION: return "Note";
	default: return "Unknown error severity";
	}
}
void GLAPIENTRY msg_callback(GLenum source, GLenum type,
		GLuint id, GLenum severity,
		GLsizei length, const GLchar* message,
		const void* userParam)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
		fprintf(stderr, "%s\t %s, type: %s, source: %s\n",
		get_err_severity(severity), message, get_err_type(type), get_err_source(source));
	}
} 

void init_celestial_sphere()
{
	float pos[][2] = {
		{ -1.0f, -1.0f },
		{  1.0f, -1.0f },
		{ -1.0f,  1.0f },
		{  1.0f,  1.0f },
	};
	unsigned int elements[] = {
		0, 2, 1,
		1, 2, 3
	};

	glGenVertexArrays(1, &glBgVerts);
	glGenBuffers(1, &glBgPosBuf);
	glGenBuffers(1, &glBgElementBuf);


	glBindVertexArray(glBgVerts);

        glBindBuffer(GL_ARRAY_BUFFER, glBgPosBuf);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBgElementBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(elements), elements,
			GL_STATIC_DRAW);

	glBindVertexArray(0);

	glGenTextures(1, &glBgTex);
	glBindTexture(GL_TEXTURE_2D, glBgTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/* Seed is generated as microseconds since 01/01/1970
 * (rounded to full seconds, i.e. 1,000,000 microseconds)
 * added with the microseconds since this program was executed.
 * This way the difference between seeds of different
 * cluster generations should be big enough.
 * Return type is uvec2, because opengl
 * does not support 64-bit integers
 */
void gen_rng_seed(unsigned int *seed1, unsigned int *seed2)
{
	time_t dt1 = time(NULL);
	clock_t dt2 = clock();
	unsigned long seedFull = (unsigned long)dt1 * 1000000UL
		+ (unsigned long)dt2 * 1000000UL
		/ (unsigned long)CLOCKS_PER_SEC;

	*seed1 = seedFull >> (8 * sizeof(unsigned int));
	*seed2 = seedFull & UINT_MAX;
}
void gen_uniform_samples(int n_samples, GLuint *_sample_buf)
{
	GLuint sample_buf;
	glGenBuffers(1, &sample_buf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, sample_buf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			n_samples * sizeof(float),
			NULL,
			GL_STATIC_DRAW);

	unsigned int seed1, seed2;
	gen_rng_seed(&seed1, &seed2);

	glUseProgram(programs.gen_uniform_samples);
	glUniform1ui(0, n_samples);
	glUniform2ui(1, seed1, seed2);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sample_buf);

	glDispatchCompute(0x10 / 0x4, 0x10 / 0x4, 0x10 / 0x4);
	glUseProgram(0);

	*_sample_buf = sample_buf;
}
void gen_gaussian_samples(int n_samples, GLuint *_sample_buf)
{
	GLuint sample_buf;
	glGenBuffers(1, &sample_buf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, sample_buf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			n_samples * sizeof(float),
			NULL,
			GL_STATIC_DRAW);

	unsigned int seed1, seed2;
	gen_rng_seed(&seed1, &seed2);

	glUseProgram(programs.gen_gaussian_samples);
	glUniform1ui(0, n_samples);
	glUniform2ui(1, seed1, seed2);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sample_buf);

	glDispatchCompute(0x10 / 0x4, 0x10 / 0x4, 0x10 / 0x4);
	glUseProgram(0);

	*_sample_buf = sample_buf;
}

void elliptic_cluster_compute_pos(struct ptcluster *cluster,
		float b, float eps, mat4 rot, float dr, float dz)
{
	GLuint usample_buf, nsample_buf;
	gen_uniform_samples(cluster->n_verts, &usample_buf);
	gen_gaussian_samples(5 * cluster->n_verts, &nsample_buf);

	glGenBuffers(1, &cluster->position_buf);
	glGenBuffers(1, &cluster->velocity_buf);
	glGenBuffers(1, &cluster->angmomentum_buf);
	glGenBuffers(1, &cluster->energy_buf);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, cluster->position_buf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			cluster->n_verts * sizeof(vec4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, cluster->velocity_buf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			cluster->n_verts * sizeof(vec4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, cluster->angmomentum_buf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			cluster->n_verts * sizeof(float), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, cluster->energy_buf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			cluster->n_verts * sizeof(float), NULL, GL_STATIC_DRAW);

	glUseProgram(programs.gen_elliptic_cluster_pos);
	glUniform1ui(0, cluster->n_verts);
	glUniform1f(1, b);
	glUniform1f(2, eps);
	glUniformMatrix4fv(3, 1, GL_FALSE, &rot[0][0]);
	glUniform1f(4, dr);
	glUniform1f(5, dz);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, usample_buf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, nsample_buf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cluster->position_buf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, cluster->velocity_buf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, cluster->angmomentum_buf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cluster->energy_buf);
	glDispatchCompute(0x10 / 0x4, 0x10 / 0x4, 0x10 / 0x4);
	glUseProgram(0);
}
void elliptic_cluster_compute_colors(struct ptcluster *cluster,
		int nBlurs, vec4 *palette, float* blurSizes)
{
	GLuint usample_buf;
	gen_uniform_samples(cluster->n_verts, &usample_buf);

	glGenBuffers(1, &cluster->color_buf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, cluster->color_buf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, cluster->n_verts * sizeof(vec4),
			NULL, GL_STATIC_DRAW);

	GLuint paletteBuf, blurSizesBuf;
	glGenBuffers(1, &paletteBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, paletteBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nBlurs * sizeof(vec4),
			palette, GL_STATIC_DRAW);
	glGenBuffers(1, &blurSizesBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, blurSizesBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nBlurs * sizeof(float),
			blurSizes, GL_STATIC_DRAW);

	glUseProgram(programs.gen_elliptic_cluster_colors);
	glUniform1ui(0, cluster->n_verts);
	glUniform1ui(1, nBlurs);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, paletteBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blurSizesBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, usample_buf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, cluster->color_buf);
	glDispatchCompute(0x10 / 0x4, 0x10 / 0x4, 0x10 / 0x4);
	glUseProgram(0);
}

void orbs_init()
{
	if (state != STATE_CLOSED) {
		errLast = ERR_WRONG_STATE;
		return;
	}

	glewInit();

	/* Only used for debug purposes
	   Can maybe used later in a debug mode?*/
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(msg_callback, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (compile_gl_programs(&programs)) {
		errLast = ERR_INTERNAL;
		return;
	}

	printf("programs:\n");
	printf(".render_cluster: %u\n", programs.render_cluster);
	printf(".evolve_cluster: %u\n", programs.evolve_cluster);
	printf(".gen_elliptic_cluster_pos: %u\n", programs.gen_elliptic_cluster_pos);
	printf(".gen_elliptic_cluster_colors: %u\n", programs.gen_elliptic_cluster_colors);
	printf(".render_celestial_sphere: %u\n", programs.render_celestial_sphere);
	printf(".gen_uniform_samples: %u\n", programs.gen_uniform_samples);
	printf(".gen_gaussian_samples: %u\n", programs.gen_gaussian_samples);
	printf("\n");

	init_celestial_sphere();
	clusters = malloc(CLUSTER_CAPACITY * sizeof(struct ptcluster));

	glUseProgram(programs.render_cluster);
	glUniform1i(4, 5);
	glUseProgram(0);

	state = STATE_INIT;
}
void orbs_close()
{
	if ((state & STATE_INIT) == 0) {
		errLast = ERR_WRONG_STATE;
		return;
	}

	orbs_remove_clusters();
	free(clusters);

	glDeleteTextures(1, &glBgTex);
	glDeleteTextures(1, &glBgRenderTex);

	glDeleteBuffers(1, &glBgPosBuf);
	glDeleteBuffers(1, &glBgElementBuf);
	glDeleteVertexArrays(1, &glBgVerts);

	free_gl_programs(&programs);

	state = STATE_CLOSED;
}
int orbs_get_last_error()
{
	return errLast;
}

void orbs_set_observer(struct Observer* obs)
{
	observer = *obs;
}
void orbs_get_observer(struct Observer* obs)
{
	*obs = observer;
}
void orbs_update_observer()
{
	if ((state & STATE_INIT) == 0) {
		errLast = ERR_WRONG_STATE;
		return;
	}

	glm_perspective(observer.fov, observer.aspect,
			observer.z_near, observer.z_far, P);
	
	/* defining 'inverse' of P */
	glm_mat2_zero(P_I);
	P_I[0][0] = 1.0f / P[0][0];
	P_I[1][1] = 1.0f / P[1][1];

	/* Camera is existing after first call of updateCamera */
	if ((state & STATE_HAS_OBSERVER) == 0) {
		state = state | STATE_HAS_OBSERVER;
	}

	orbs_update_observer_vectors();
}
void orbs_update_observer_vectors()
{
	if ((state & (STATE_INIT | STATE_HAS_OBSERVER)) == 0) {
		errLast = ERR_WRONG_STATE;
		return;
	}

	vec3 obs_center;
	glm_vec3_add(observer.pos, observer.look_dir, obs_center);
	glm_lookat(observer.pos, obs_center, observer.up_dir, V);

	mat4 PV;
	glm_mat4_mul(P, V, PV);

	glUseProgram(programs.render_celestial_sphere);
	glUniform4fv(0, 1, &observer.pos[0]);
	glUniformMatrix2fv(1, 1, GL_FALSE, &P_I[0][0]);
	glUniformMatrix4fv(2, 1, GL_TRUE, &V[0][0]);
	glUseProgram(0);

	glUseProgram(programs.render_cluster);
	glUniformMatrix4fv(0, 1, GL_FALSE, &PV[0][0]);
	glUniform4fv(1, 1, &observer.pos[0]);
	glUseProgram(0);
}

void orbs_set_celestial_sphere(struct CelestialSphere* csphere)
{
	celestial_sphere = *csphere;
}
void orbs_get_celestial_sphere(struct CelestialSphere* csphere)
{
	*csphere = celestial_sphere;
}
void orbs_update_celestial_sphere()
{
	if ((state & STATE_INIT) == 0) {
		errLast = ERR_WRONG_STATE;
		return;
	}

	glBindTexture(GL_TEXTURE_2D, glBgTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			celestial_sphere.imgwidth, celestial_sphere.imgheight,
			0, GL_RGB, GL_UNSIGNED_BYTE, celestial_sphere.imgdata);

	state = state | STATE_HAS_BG_TEXTURE;
}
void orbs_update_celesital_sphere_imgdata()
{
	if ((state & (STATE_INIT | STATE_HAS_BG_TEXTURE)) == 0) {
		errLast = ERR_WRONG_STATE;
		return;
	}

	glBindTexture(GL_TEXTURE_2D, glBgTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			celestial_sphere.imgwidth, celestial_sphere.imgheight,
			GL_RGB, GL_UNSIGNED_BYTE, celestial_sphere.imgdata);
}

void orbs_render()
{
	struct timespec t0 = {0, 0};
	struct timespec t1 = {0, 0};
	struct timespec t2 = {0, 0};
	struct timespec t3 = {0, 0};
	struct timespec t4 = {0, 0};
	struct timespec t5 = {0, 0};

	clock_gettime(CLOCK_MONOTONIC, &t0);

	if ((state & STATE_INIT) == 0) {
		errLast = ERR_WRONG_STATE;
		return;
	}

	clock_gettime(CLOCK_MONOTONIC, &t1);

	//glClear(GL_COLOR_BUFFER_BIT);
	//glFinish(); /* Why? */

	glUseProgram(programs.render_celestial_sphere);
	glBindVertexArray(glBgVerts);
  	glBindTexture(GL_TEXTURE_2D, glBgTex);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
	glUseProgram(0);

	clock_gettime(CLOCK_MONOTONIC, &t2);

	//glFinish(); /* Why? */
	//glMemoryBarrier(GL_ALL_BARRIER_BITS); /* Why? */

	glUseProgram(programs.render_cluster);
	//glMemoryBarrier(GL_UNIFORM_BARRIER_BIT); /* Why? */
	//glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT); /* Why? */
	for (int i = 0; i < nClusters; i++) {
		glBindVertexArray(clusters[i].vert_array);
		//glFinish(); /* Why? */

		clock_gettime(CLOCK_MONOTONIC, &t3);

		glUniform1i(2, 1);
		glUniform1ui(3, 0);
		glDrawArrays(GL_POINTS, 0, clusters[i].n_verts);

		glUniform1ui(3, 1);
		glDrawArrays(GL_POINTS, 0, clusters[i].n_verts);

		glUniform1ui(2, 0);
		glUniform1ui(3, 0);
		glDrawArrays(GL_POINTS, 0, clusters[i].n_verts);

		glUniform1ui(3, 1);
		glDrawArrays(GL_POINTS, 0, clusters[i].n_verts);

		clock_gettime(CLOCK_MONOTONIC, &t4);

		glBindVertexArray(0);
	}
	glUseProgram(0);

	clock_gettime(CLOCK_MONOTONIC, &t5);

	double dt1 = ((double)t1.tv_sec + 1e-9 * t1.tv_nsec)
		- ((double)t0.tv_sec + 1e-9 * t0.tv_nsec);
	double dt2 = ((double)t2.tv_sec + 1e-9 * t2.tv_nsec)
		- ((double)t1.tv_sec + 1e-9 * t1.tv_nsec);
	double dt3 = ((double)t3.tv_sec + 1e-9 * t3.tv_nsec)
		- ((double)t2.tv_sec + 1e-9 * t2.tv_nsec);
	double dt4 = ((double)t4.tv_sec + 1e-9 * t4.tv_nsec)
		- ((double)t3.tv_sec + 1e-9 * t3.tv_nsec);
	double dt5 = ((double)t5.tv_sec + 1e-9 * t5.tv_nsec)
		- ((double)t4.tv_sec + 1e-9 * t4.tv_nsec);
	//printf("dt1 = %.10f s\n", dt1);
	//printf("dt2 = %.10f s\n", dt1);
	//printf("dt3 = %.10f s\n", dt1);
	//printf("dt4 = %.10f s\n", dt1);
	//printf("dt5 = %.10f s\n", dt1);
	//printf("\n");
}
void orbs_update_global_time(float t)
{
	/* glEnable(GL_DEBUG_OUTPUT);
	   glDebugMessageCallback(msgCallback, 0); */

	glFinish(); /* Why? */

	float dt = t - globalTime;
	glUseProgram(programs.evolve_cluster);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); /* Why? */
	for (int i = 0; i < nClusters; ++i) {
		glUniform1ui(0, clusters[i].n_verts);
		glUniform1f(1, dt);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
				0, clusters[i].position_buf);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
				1, clusters[i].velocity_buf);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
				2, clusters[i].angmomentum_buf);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
				3, clusters[i].energy_buf);

		glFinish(); /* Why? */

		glDispatchCompute(0x10 / 0x4, 0x10 / 0x4, 0x10 / 0x4);
	}
	glUseProgram(0);
	globalTime = t;
}

void orbs_create_elliptic_cluster(int n_particles,
		float a, float b, vec3 n, float dr, float dz,
		int nBlurs, vec4 *palette, float* blurSizes)
{
	if ((state & STATE_INIT) == 0) {
		errLast = ERR_WRONG_STATE;
		return;
	}

	glFinish(); /* Why? */

	struct ptcluster cluster;
	cluster.n_verts = n_particles;

	float eps;
	float angle;
	vec3 axis;
	mat4 rot;
	vec4 up = { 0.0f, 0.0f, 1.0f };

	eps = sqrtf(1.0f - b * b / (a * a));
	angle = glm_vec3_dot(up, n) / glm_vec3_norm(n);
	glm_vec3_cross(up, n, axis);
	glm_rotate_make(rot, angle, axis);

	elliptic_cluster_compute_pos(&cluster, b, eps, rot, dr, dz);

	elliptic_cluster_compute_colors(&cluster, nBlurs, palette, blurSizes);

	//glFinish();
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, cluster.position_buf);
	//vec4 *test_buf = (vec4 *)glMapBuffer(GL_SHADER_STORAGE_BUFFER,
	//					GL_READ_ONLY);
	//for (int i = 0; i < 100; ++i) {
	//	int j = i * (n_particles - 1) / 100;
	//	printf("(%f %f %f %f)\n",
	//			test_buf[j][0],
	//			test_buf[j][1],
	//			test_buf[j][2],
	//			test_buf[j][3]);
	//}
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//getchar();

	glGenVertexArrays(1, &cluster.vert_array);
	glBindVertexArray(cluster.vert_array);
	glBindBuffer(GL_ARRAY_BUFFER, cluster.position_buf);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, sizeof(vec4) / sizeof(float),
			GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, cluster.color_buf);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, sizeof(vec4) / sizeof(float),
			GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(0);

	clusters[nClusters] = cluster;
	++nClusters;

	glFinish(); /* Why? */
}
void orbs_remove_clusters()
{
	for (int i = 0; i < nClusters; i++) {
		glDeleteBuffers(1, &clusters[i].position_buf);
		glDeleteBuffers(1, &clusters[i].color_buf);
		glDeleteBuffers(1, &clusters[i].velocity_buf);
		glDeleteBuffers(1, &clusters[i].angmomentum_buf);
		glDeleteBuffers(1, &clusters[i].energy_buf);
		glDeleteVertexArrays(1, &clusters[i].vert_array);
	}
	nClusters = 0;
}

