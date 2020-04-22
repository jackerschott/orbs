#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <GL/glew.h>
#include <GL/gl.h>

/* shader compilation functions */
#define SC_MAX_SHADERS_PER_PROG 2
#include "sc.h"

#include "simulation.h"

/* Shader source code as strings */
#include "shadersrc.h"

/* Shader locations of arguments */
#define LOC_BG_C 			0
#define LOC_BG_P_I 			1
#define LOC_BG_V 			2

#define LOC_CLUSTER_P 			0
#define LOC_CLUSTER_COLOR_VS 		1
#define LOC_CLUSTER_PV  		0
#define LOC_CLUSTER_C  			1
#define LOC_CLUSTER_INVERT 		2
#define LOC_CLUSTER_N_LOOPS 		3

#define LOC_CLUSTERDYN_N_PT 		0
#define LOC_CLUSTERDYN_DT 		1
#define LOC_CLUSTERDYN_POS_BUF 		0
#define LOC_CLUSTERDYN_VEL_BUF 		1
#define LOC_CLUSTERDYN_L_BUF 		2
#define LOC_CLUSTERDYN_E_BUF 		3

#define LOC_RNG_UNIFORM_N_SAMPLES 	0
#define LOC_RNG_UNIFORM_OFF 		1
#define LOC_RNG_UNIFORM_SAMPLES_BUF 	0

#define LOC_RNG_GAUSS_N_SAMPLES 	0
#define LOC_RNG_GAUSS_OFF 		1
#define LOC_RNG_GAUSS_SAMPLES_BUF 	0

#define LOC_CLUSTERPOS_N_PT 		0
#define LOC_CLUSTERPOS_P 		1
#define LOC_CLUSTERPOS_EPS 		2
#define LOC_CLUSTERPOS_ROT 		3
#define LOC_CLUSTERPOS_DR 		4
#define LOC_CLUSTERPOS_DZ 		5
#define LOC_CLUSTERPOS_U_SAMPLES_BUF 	0
#define LOC_CLUSTERPOS_N_SAMPLES_BUF 	1
#define LOC_CLUSTERPOS_POS_BUF 		2
#define LOC_CLUSTERPOS_VEL_BUF 		3
#define LOC_CLUSTERPOS_L_BUF 		4
#define LOC_CLUSTERPOS_E_BUF 		5

#define LOC_CLUSTERCOL_N_PT 		0
#define LOC_CLUSTERCOL_N_COLORS 	1
#define LOC_CLUSTERCOL_PALETTE_BUF 	0
#define LOC_CLUSTERCOL_BLUR_SIZES_BUF 	1
#define LOC_CLUSTERCOL_U_SAMPLES_BUF 	2
#define LOC_CLUSTERCOL_COLORS_BUF 	3

/* Compute shader parameters */
#define CLUSTERDYN_INVOCATIONS_1D 	0x10
#define CLUSTERDYN_WORK_GROUP_SIZE_1D 	0x4
#define CLUSTERDYN_NUM_WORK_GROUPS_1D \
	(CLUSTERDYN_INVOCATIONS_1D / CLUSTERDYN_WORK_GROUP_SIZE_1D)

#define RNG_UNIFORM_INVOCATIONS_1D 	0x10
#define RNG_UNIFORM_WORK_GROUP_SIZE_1D 	0x4
#define RNG_UNIFORM_NUM_WORK_GROUPS_1D \
	(RNG_UNIFORM_INVOCATIONS_1D / RNG_UNIFORM_WORK_GROUP_SIZE_1D)

#define RNG_GAUSS_INVOCATIONS_1D	0x10
#define RNG_GAUSS_WORK_GROUP_SIZE_1D 	0x4
#define RNG_GAUSS_NUM_WORK_GROUPS_1D \
	(RNG_GAUSS_INVOCATIONS_1D / RNG_GAUSS_WORK_GROUP_SIZE_1D)

#define CLUSTERPOS_INVOCATIONS_1D	0x10
#define CLUSTERPOS_WORK_GROUP_SIZE_1D 	0x4
#define CLUSTERPOS_NUM_WORK_GROUPS_1D \
	(CLUSTERPOS_INVOCATIONS_1D / CLUSTERPOS_WORK_GROUP_SIZE_1D)

#define CLUSTERCOL_INVOCATIONS_1D	0x10
#define CLUSTERCOL_WORK_GROUP_SIZE_1D 	0x4
#define CLUSTERCOL_NUM_WORK_GROUPS_1D \
	(CLUSTERCOL_INVOCATIONS_1D / CLUSTERCOL_WORK_GROUP_SIZE_1D)

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

/* Some math constants (for some reason i did not use those in math.h
 * probably precision or the consistency because i wanted 2*PI to be defined)
 */
#define PI      3.14159265358979323846
#define PI2     6.28318530717958647693
#define PI_2    1.57079632679489661923

/* Opengl shader compilation */
#define N_PROGS 7
/* Access opengl programs through union to automate compilation, while
   maintaining the possiblity to address each program individually */
union {
	struct {
		GLuint rngUniform;
		GLuint rngGauss;
		GLuint clusterpos;
		GLuint clustercol;
		GLuint bg;
		GLuint cluster;
		GLuint clusterdyn;
	};
	GLuint progs[N_PROGS];
} progs;
const char* progNames[] = {
	"rngUniform",
	"rngGauss",
	"clusterpos",
	"clustercol",
	"bg",
	"cluster",
	"clusterdyn",
};
int nShaders[] = { 1, 1, 1, 1, 2, 2, 1 };
GLuint shaderTypes[][SC_MAX_SHADERS_PER_PROG] = {
	{ GL_COMPUTE_SHADER },
	{ GL_COMPUTE_SHADER },
	{ GL_COMPUTE_SHADER },
	{ GL_COMPUTE_SHADER },
	{ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER },
	{ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER },
	{ GL_COMPUTE_SHADER },
};
const char *shadersrcs[][SC_MAX_SHADERS_PER_PROG] = {
	{ RNG_UNIFORM_COMP_SRC },
	{ RNG_GAUSS_COMP_SRC },
	{ CLUSTERPOS_COMP_SRC },
	{ CLUSTERCOL_COMP_SRC },
	{ BG_VERT_SRC, BG_FRAG_SRC },
	{ CLUSTER_VERT_SRC, CLUSTER_FRAG_SRC },
	{ CLUSTERDYN_COMP_SRC },
};
const char* shaderNames[][SC_MAX_SHADERS_PER_PROG] = {
	{ "rng_uniform_comp" },
	{ "rng_gauss_comp" },
	{ "clusterpos_comp" },
	{ "clustercol_comp" },
	{ "bg_vert", "bg_frag" },
	{ "cluster_vert", "cluster_frag" },
	{ "clusterdyn_comp" },
};

/* Particle cluster rendering */
#ifndef CLUSTER_CAPACITY
#define CLUSTER_CAPACITY 16
#endif

int nClusters = 0;
int nClusterVerts[CLUSTER_CAPACITY];
GLuint glClusterVerts[CLUSTER_CAPACITY];
GLuint glClusterPosBufs[CLUSTER_CAPACITY];
GLuint glClusterColBufs[CLUSTER_CAPACITY];
GLuint glClusterVelBufs[CLUSTER_CAPACITY];
GLuint glClusterLBufs[CLUSTER_CAPACITY];
GLuint glClusterEBufs[CLUSTER_CAPACITY];

float globalTime;

/* Camera */
struct slCamera cam;
mat4 P; /* Projection matrix */
mat2 P_I; /* 'inverse' of P, needed for bg rendering */
mat4 V; /* View rotation matrix */

/* Background rendering */
unsigned int bgTexWidth;
unsigned int bgTexHeight;
char *bgTexData;

GLuint glBgVerts;
GLuint glBgPosBuf;
GLuint glBgElementBuf;
GLuint glBgTex;
GLuint glBgRenderTex;

/* Debugging and error handling*/
slState state = SL_STATE_CLOSED;
int errLast;

int compilePrograms();
void handleProgramCompileError(int err, int progIndex);
void genRngSeed(unsigned int* seed1, unsigned int* seed2);

/* Only used for debug purposes */
void GLAPIENTRY msgCallback(GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length,
		const GLchar* message, const void* userParam);
const char* getErrSource(GLenum source);
const char* getErrType(GLenum type);
const char* getErrSeverity(GLenum severity);

void slInit() {
	if (state != SL_STATE_CLOSED) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}

	glewInit();

	/* Only used for debug purposes
	   Can maybe used later in a debug mode?*/
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(msgCallback, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (!compilePrograms()) {
		errLast = SL_ERR_INTERNAL;
		return;
	}

	/* Initialize objects for background rendering */
	glGenVertexArrays(1, &glBgVerts);
	glGenBuffers(1, &glBgPosBuf);
	glGenBuffers(1, &glBgElementBuf);

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

	glBindVertexArray(glBgVerts);

        glBindBuffer(GL_ARRAY_BUFFER, glBgPosBuf);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT,
			false, /* normalization */
			0, /* stride */
			NULL); /* offset */

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBgElementBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	glBindVertexArray(0);

	glGenTextures(1, &glBgTex);
	glBindTexture(GL_TEXTURE_2D, glBgTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	state = SL_STATE_INIT;
}
void slClose() {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}

	slClearClusters();

	glDeleteTextures(1, &glBgTex);
	glDeleteTextures(1, &glBgRenderTex);

	glDeleteBuffers(1, &glBgPosBuf);
	glDeleteBuffers(1, &glBgElementBuf);
	glDeleteVertexArrays(1, &glBgVerts);

	for (int i = 0; i < N_PROGS; ++i) {
		scFreeProgram(progs.progs[i]);
	}

	state = SL_STATE_CLOSED;
}
slState slGetState() {
        return state;
}
int slGetLastError() {
	return errLast;
}

void slGetCameraPos(vec3 pos) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	memcpy(pos, cam.pos, sizeof(vec3));
}
void slGetCameraLookDir(vec3 lookDir) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	memcpy(lookDir, cam.lookDir, sizeof(vec3));
}
void slGetCameraUpDir(vec3 upDir) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	memcpy(upDir, cam.upDir, sizeof(vec3));
}
float slGetCameraFov() {
	return cam.fov;
}
float slGetCameraAspect() {
	return cam.aspect;
}
float slGetCameraZNear() {
	return cam.zNear;
}
float slGetCameraZFar() {
	return cam.zFar;
}
void slSetCamera(vec3 pos, vec3 lookDir, vec3 upDir,
		float fov, float aspect, float zNear, float zFar) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	memcpy(cam.pos, pos, sizeof(vec3));
	memcpy(cam.lookDir, lookDir, sizeof(vec3));
	memcpy(cam.upDir, upDir, sizeof(vec3));
	cam.fov = fov;
	cam.aspect = aspect;
	cam.zNear = zNear;
	cam.zFar = zFar;
}
void slSetCameraView(vec3 pos, vec3 lookDir) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	memcpy(cam.pos, pos, sizeof(vec3));
	memcpy(cam.lookDir, lookDir, sizeof(vec3));
}
void slSetCameraPos(vec3 pos) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	memcpy(cam.pos, pos, sizeof(vec3));
}
void slSetCameraLookDir(vec3 lookDir) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	memcpy(cam.lookDir, lookDir, sizeof(vec3));
}
void slSetCameraUpDir(vec3 upDir) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	memcpy(cam.upDir, upDir, sizeof(vec3));
}
void slSetCameraFov(float fov) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	cam.fov = fov;
}
void slSetCameraAspect(float aspect) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	cam.aspect = aspect;
}
void slSetCameraZNear(float zNear) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	cam.zNear = zNear;
}
void slSetCameraZFar(float zFar) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	cam.zFar = zFar;
}
void slUpdateCamera() {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}

	glm_perspective(cam.fov, cam.aspect, cam.zNear, cam.zFar, P);
	
	/* defining 'inverse' of P */
	glm_mat2_zero(P_I);
	P_I[0][0] = 1.0f / P[0][0];
	P_I[1][1] = 1.0f / P[1][1];

	/* Camera is existing after first call of updateCamera */
	if ((state & SL_STATE_HAS_CAM) == 0) {
		state = state | SL_STATE_HAS_CAM;
	}

	slUpdateCameraView();
}
void slUpdateCameraView() {
	if ((state & (SL_STATE_INIT | SL_STATE_HAS_CAM)) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}

	vec3 camCenter;
	glm_vec3_add(cam.pos, cam.lookDir, camCenter);
	glm_lookat(cam.pos, camCenter, cam.upDir, V);

	mat4 PV;
	glm_mat4_mul(P, V, PV);

	glUseProgram(progs.bg);
	glUniform4fv(LOC_BG_C, 1, &cam.pos[0]);
	glUniformMatrix2fv(LOC_BG_P_I, 1, false, &P_I[0][0]);
	/* True transposition argument, because the
	   inverse/transpose (rotation matrix) of V is needed */
	glUniformMatrix4fv(LOC_BG_V, 1, true, &V[0][0]);
	glUseProgram(0);

	glUseProgram(progs.cluster);
	glUniformMatrix4fv(LOC_CLUSTER_PV, 1, false, &PV[0][0]);
	glUniform4fv(LOC_CLUSTER_C, 1, &cam.pos[0]);
	glUseProgram(0);
}

void slSetBackgroundTex(unsigned int width, unsigned int height,
		char* data) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	slSetBackgroundTexSize(width, height);
	slSetBackgroundTexData(data);
}
void slSetBackgroundTexSize(unsigned int width, unsigned int height) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	bgTexWidth = width;
	bgTexHeight = height;
}
void slSetBackgroundTexData(char* data) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}
	bgTexData = data;
}
void slUpdateBackgroundTex() {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}

	GLint levelDetail = 0;
	GLint internalformat = GL_RGBA32F; /* Why? */
	GLint border = 0; /* Must be 0 */
	GLint format = GL_BGRA; /* Why? Easiest to load form file? */
	GLenum type = GL_UNSIGNED_BYTE;

	glBindTexture(GL_TEXTURE_2D, glBgTex);
	glTexImage2D(GL_TEXTURE_2D,
			levelDetail, internalformat,
			bgTexWidth, bgTexHeight,
			border, format,
			type, bgTexData);

	state = state | SL_STATE_HAS_BG_TEXTURE;
}
void slUpdateBackgroundTexData() {
	if ((state & (SL_STATE_INIT | SL_STATE_HAS_BG_TEXTURE)) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}

	GLint levelDetail = 0;
	GLint xoffset = 0;
	GLint yoffset = 0;
	GLint format = GL_RGBA;
	GLint border = 0;
	GLenum type = GL_UNSIGNED_BYTE;

	glBindTexture(GL_TEXTURE_2D, glBgTex);
	glTexSubImage2D(GL_TEXTURE_2D,
			levelDetail,
			xoffset, yoffset,
			bgTexWidth, bgTexHeight,
			format,
			type, bgTexData);
}

void slRenderClassic() {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}

	glFinish(); /* Why? */


	/* Render background */
	//glUseProgram(progs.bg);
	//glBindVertexArray(glBgVerts);

  	//glBindTexture(GL_TEXTURE_2D, glBgTex);

	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	//glBindVertexArray(0);
	//glUseProgram(0);


	//glFinish(); /* Why? */
	//glMemoryBarrier(GL_ALL_BARRIER_BITS); /* Why? */

	/* Render particle clusters */
	glUseProgram(progs.cluster);
	glMemoryBarrier(GL_UNIFORM_BARRIER_BIT); /* Why? */
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT); /* Why? */
	for (int i = 0; i < nClusters; i++) {
		glBindVertexArray(glClusterVerts[i]);
		glFinish(); /* Why? */

		//glPointSize(10.0);

		/* Render clusters for different classes of light paths */
		glUniform1i(LOC_CLUSTER_INVERT, 1);
		glUniform1ui(LOC_CLUSTER_N_LOOPS, 0);
		glDrawArrays(GL_POINTS,
				0, /* Input array offset */
				nClusterVerts[i]);

		glUniform1ui(LOC_CLUSTER_N_LOOPS, 1);
		glDrawArrays(GL_POINTS,
				0, /* Input array offset */
				nClusterVerts[i]);

		glUniform1ui(LOC_CLUSTER_INVERT, 0);
		glUniform1ui(LOC_CLUSTER_N_LOOPS, 0);
		glDrawArrays(GL_POINTS,
				0, /* Input array offset */
				nClusterVerts[i]);

		glUniform1ui(LOC_CLUSTER_N_LOOPS, 1);
		glDrawArrays(GL_POINTS,
				0, /* Input array offset */
				nClusterVerts[i]);

		glBindVertexArray(0);
	}
	printf("\n");
	glUseProgram(0);
}
void slUpdateGlobalTime(float t) {
	/* glEnable(GL_DEBUG_OUTPUT);
	   glDebugMessageCallback(msgCallback, 0); */

	glFinish(); /* Why? */

	float dt = t - globalTime;
	glUseProgram(progs.clusterdyn);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); /* Why? */
	for (int i = 0; i < nClusters; ++i) {
		glUniform1ui(LOC_CLUSTERDYN_N_PT, nClusterVerts[i]);
		glUniform1f(LOC_CLUSTERDYN_DT, dt);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
				LOC_CLUSTERDYN_POS_BUF,
				glClusterPosBufs[i]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
				LOC_CLUSTERDYN_VEL_BUF,
				glClusterVelBufs[i]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
				LOC_CLUSTERDYN_L_BUF,
				glClusterLBufs[i]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
				LOC_CLUSTERDYN_E_BUF,
				glClusterEBufs[i]);

		glFinish(); /* Why? */

		glDispatchCompute(CLUSTERDYN_WORK_GROUP_SIZE_1D,
				  CLUSTERDYN_WORK_GROUP_SIZE_1D,
				  CLUSTERDYN_WORK_GROUP_SIZE_1D);
	}
	glUseProgram(0);
	globalTime = t;
}

void slCreateEllipticCluster(unsigned int nParticles,
		float a, float b, vec3 n,
		float dr, float dz,
		int nBlurs, vec4 *palette, float* blurSizes) {
	if ((state & SL_STATE_INIT) == 0) {
		errLast = SL_ERR_WRONG_STATE;
		return;
	}

	printf("Compute cluster...\n");

	glFinish(); /* Why? */

	/* Time measurement, please remove */
	clock_t t1 = clock();

	/* Initialize cluster array buffers, enable attrib arrays,
	   buffer data is written in compute shaders */
	GLuint clusterVerts;
	GLuint posBuf, colorBuf;
	GLuint velBuf, LBuf, EBuf;
	glGenVertexArrays(1, &clusterVerts);

	/* Position buffer */
	glGenBuffers(1, &posBuf);
	glBindVertexArray(clusterVerts);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	glBufferData(GL_ARRAY_BUFFER,
			nParticles * sizeof(vec4),
			NULL, /* data */
			GL_STATIC_DRAW);

	glEnableVertexAttribArray(LOC_CLUSTER_P);
	glVertexAttribPointer(LOC_CLUSTER_P,
			sizeof(vec4) / sizeof(float), GL_FLOAT,
			GL_FALSE, /* normalisation */
			0, /* stride */
			NULL); /* offset */

	/* Color buffer */
	glGenBuffers(1, &colorBuf);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuf);
	glBufferData(GL_ARRAY_BUFFER,
			nParticles * sizeof(vec4),
			NULL, /* data */
			GL_STATIC_DRAW);

	glEnableVertexAttribArray(LOC_CLUSTER_COLOR_VS);
	glVertexAttribPointer(LOC_CLUSTER_COLOR_VS,
			sizeof(vec4) / sizeof(float), GL_FLOAT,
			GL_FALSE, /* normalisation */
			0, /* stride */
			NULL); /* offset */
	glBindVertexArray(0);

	/* Initialize remaining cluster buffers,
	   buffer data is writtin in compute shaders */
	glGenBuffers(1, &velBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, velBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			nParticles * sizeof(vec4),
			NULL, /* data */
			GL_STATIC_DRAW);

	glGenBuffers(1, &LBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, LBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			nParticles * sizeof(float),
			NULL, /* data */
			GL_STATIC_DRAW);

	glGenBuffers(1, &EBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, EBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			nParticles * sizeof(float),
			NULL, /* data */
			GL_STATIC_DRAW);

	/* Initialize sample buffers,
	   buffer data is written in compute shaders */
	GLuint uPosSamplesBuf, uColSamplesBuf;
	GLuint nPosSamplesBuf;
	GLuint paletteBuf, blurSizesBuf;
	glGenBuffers(1, &uPosSamplesBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, uPosSamplesBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			nParticles * sizeof(float),
			NULL, /* data */
			GL_STREAM_READ);
	glGenBuffers(1, &uColSamplesBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, uColSamplesBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			nParticles * sizeof(float),
			NULL, /* data */
			GL_STREAM_READ);
	glGenBuffers(1, &nPosSamplesBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, nPosSamplesBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			5 * nParticles * sizeof(float),
			NULL, /* data */
			GL_STREAM_READ);

	/* Initialize color generation buffers */
	glGenBuffers(1, &paletteBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, paletteBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			nBlurs * sizeof(vec4),
			palette,
			GL_STREAM_READ);
	glGenBuffers(1, &blurSizesBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, blurSizesBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
			nBlurs * sizeof(float),
			blurSizes,
			GL_STREAM_READ);

	/* Generate some random samples of respective distributions
	   These are needed to determine the parameters of each particle */
	unsigned int seed1, seed2;
	genRngSeed(&seed1, &seed2);
	glUseProgram(progs.rngUniform);
	glUniform1ui(LOC_RNG_UNIFORM_N_SAMPLES, nParticles);
	glUniform2ui(LOC_RNG_UNIFORM_OFF, seed1, seed2);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_RNG_UNIFORM_SAMPLES_BUF, uPosSamplesBuf);
	glDispatchCompute(RNG_UNIFORM_WORK_GROUP_SIZE_1D,
			  RNG_UNIFORM_WORK_GROUP_SIZE_1D,
			  RNG_UNIFORM_WORK_GROUP_SIZE_1D);

	genRngSeed(&seed1, &seed2);
	glUniform2ui(LOC_RNG_UNIFORM_OFF, seed1, seed2);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_RNG_UNIFORM_SAMPLES_BUF, uColSamplesBuf);
	glDispatchCompute(RNG_UNIFORM_WORK_GROUP_SIZE_1D,
			  RNG_UNIFORM_WORK_GROUP_SIZE_1D,
			  RNG_UNIFORM_WORK_GROUP_SIZE_1D);
	glUseProgram(0);

	genRngSeed(&seed1, &seed2);
	glUseProgram(progs.rngGauss);
	glUniform1ui(LOC_RNG_GAUSS_N_SAMPLES, 5 * nParticles);
	glUniform2ui(LOC_RNG_GAUSS_OFF, seed1, seed2);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_RNG_GAUSS_SAMPLES_BUF, nPosSamplesBuf);
	glDispatchCompute(RNG_GAUSS_WORK_GROUP_SIZE_1D,
			  RNG_GAUSS_WORK_GROUP_SIZE_1D,
			  RNG_GAUSS_WORK_GROUP_SIZE_1D);
	glUseProgram(0);

	glFinish(); /* Why? */
	clock_t t2 = clock();

	mat4 rot;
	float angle = n[2] / glm_vec3_norm(n);
	vec3 axis = { -n[1], n[0], 0.0f };
	float eps = sqrt(1.0f - b * b / (a * a));
	glm_rotate_make(rot, angle, axis);

	glUseProgram(progs.clusterpos);
	glUniform1ui(LOC_CLUSTERPOS_N_PT, nParticles);
	glUniform1f(LOC_CLUSTERPOS_P, b);
	glUniform1f(LOC_CLUSTERPOS_EPS, eps);
	glUniformMatrix4fv(LOC_CLUSTERPOS_ROT, 1, false, &rot[0][0]);
	glUniform1f(LOC_CLUSTERPOS_DR, dr);
	glUniform1f(LOC_CLUSTERPOS_DZ, dz);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_CLUSTERPOS_U_SAMPLES_BUF, uPosSamplesBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 
			LOC_CLUSTERPOS_N_SAMPLES_BUF, nPosSamplesBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_CLUSTERPOS_POS_BUF, posBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_CLUSTERPOS_VEL_BUF, velBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_CLUSTERPOS_L_BUF, LBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_CLUSTERPOS_E_BUF, EBuf);
	glDispatchCompute(0x10 / 4,
			  0x10 / 4,
			  0x10 / 4);
	glUseProgram(0);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, posBuf);
	//vec4 *testBuf = (vec4 *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//for (int i = 0; i < 100; ++i) {
	//	int j = i * (nParticles - 1) / 100;
	//	printf("(%f %f %f %f)\n", testBuf[i][0], testBuf[i][1], testBuf[i][2], testBuf[i][3]);
	//}
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//getchar();

	glUseProgram(progs.clustercol);
	glUniform1ui(LOC_CLUSTERCOL_N_PT, nParticles);
	glUniform1ui(LOC_CLUSTERCOL_N_COLORS, nBlurs);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_CLUSTERCOL_PALETTE_BUF, paletteBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_CLUSTERCOL_BLUR_SIZES_BUF, blurSizesBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_CLUSTERCOL_U_SAMPLES_BUF, uColSamplesBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
			LOC_CLUSTERCOL_COLORS_BUF, colorBuf);
	glDispatchCompute(0x10 / 4,
			  0x10 / 4,
			  0x10 / 4);
	glUseProgram(0);

	nClusterVerts[nClusters] = nParticles;
	glClusterVerts[nClusters] = clusterVerts;
	glClusterPosBufs[nClusters] = posBuf;
	glClusterColBufs[nClusters] = colorBuf;
	glClusterVelBufs[nClusters] = velBuf;
	glClusterLBufs[nClusters] = LBuf;
	glClusterEBufs[nClusters] = EBuf;
	nClusters++;

	glFinish(); /* Why? */
	clock_t t3 = clock();
	float dt1 = ((float)(t2 - t1)) / CLOCKS_PER_SEC;
	float dt2 = ((float)(t3 - t2)) / CLOCKS_PER_SEC;
	float dt = ((float)(t3 - t1)) / CLOCKS_PER_SEC;
	printf("Done\n");
	printf("Sample computation time for %u particles: %f s\n",
			nParticles, dt1);
	printf("Cluster computation time for %u particles: %f s\n",
			nParticles, dt2);
	printf("Total computation time for %u particles: %f s\n",
			nParticles, dt);
	printf("\n");
}
void slClearClusters() {
	for (int i = 0; i < nClusters; i++) {
		glDeleteBuffers(1, &glClusterPosBufs[i]);
		glDeleteBuffers(1, &glClusterColBufs[i]);
		glDeleteVertexArrays(1, &glClusterVerts[i]);
	}
	nClusters = 0;
}

int compilePrograms() {
	for (int i = 0; i < N_PROGS; ++i) {
		GLuint prog = scCompileProgram(nShaders[i], shaderTypes[i], shadersrcs[i]);
		int err = scGetCompileStatus();
		if (err != SC_ERR_NONE) {
			handleProgramCompileError(err, i);

			scFreeShaders(prog);
			scFreeProgram(prog);
			for (int j = 0; j < i; ++j) {
				scFreeProgram(progs.progs[j]);
			}
			return 0;
		}
		scFreeShaders(prog);
		progs.progs[i] = prog;
	}
	return 1;
}
void handleProgramCompileError(int err, int progIndex) {
	GLint logLength;
	scGetErrorLogLength(&logLength);
	GLchar *log = (GLchar *)malloc(logLength);
	scGetErrorLog(logLength, NULL, log);

	printf("Internal error: ");
	switch (err) {
		case SC_ERR_SHADER_COMPILE: {
			int j = scGetErrorShaderIndex();
			printf("Shader %s failed to compile:\n", shaderNames[progIndex][j]);
			printf("%s\n", log);
			break;
	    	}
		case SC_ERR_PROG_LINK: {
			printf("Program %s failed to link:\n", progNames[progIndex]);
			printf("%s\n", log);
			break;
		}
		case SC_ERR_PROG_VALIDATE: {
			printf("Program %s could not be validated:\n", progNames[progIndex]);
			printf("%s\n", log);
			break;
		}		
	}
}
void genRngSeed(unsigned int *seed1, unsigned int *seed2) {
	/* Seed is generated as microseconds since 01/01/1970
	   (rounded to full seconds, i.e. 1,000,000 microseconds)
	   added with the microseconds since this program was executed.
	   This way the difference between seeds of different
	   cluster generations should be big enough */
	time_t dt1 = time(NULL); /* unix time */
	clock_t dt2 = clock(); /* clock cycles since execution */
	unsigned long seedFull = ((unsigned long)dt1) * 1000000UL
		+ ((unsigned long)dt2) * 1000000UL
		/ ((unsigned long)CLOCKS_PER_SEC);

	/* Return type is uvec2, because opengl does not support 64-bit integers */
	*seed1 = seedFull >> (8 * sizeof(unsigned int)); /* First 32 bits of seed */
	*seed2 = seedFull & UINT_MAX; /* Last 32 bits of seed */
}

/* Only used for debug purposes */
void GLAPIENTRY msgCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam) {
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
		fprintf(stderr, "%s\t %s, type: %s, source: %s\n",
		getErrSeverity(severity), message, getErrType(type), getErrSource(source));
	}
}
const char* getErrSource(GLenum source) {
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
const char* getErrType(GLenum type) {
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
const char* getErrSeverity(GLenum severity) {
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH: return "Error";
	case GL_DEBUG_SEVERITY_MEDIUM: return "Major warning";
	case GL_DEBUG_SEVERITY_LOW: return "Warning";
	case GL_DEBUG_SEVERITY_NOTIFICATION: return "Note";
	default: return "Unknown error severity";
	}
}
