#ifndef SIMULATON_H
#define SIMULATON_H

#include <cglm/cglm.h>

#define SL_STATE_CLOSED	    		0
#define SL_STATE_INIT	      		(1<<0)
#define SL_STATE_HAS_CAM 		(1<<1)
#define SL_STATE_HAS_BG_TEXTURE    	(1<<2)

#define SL_ERR_NONE 			0
#define SL_ERR_WRONG_STATE 		1
/* Should probably be removed in the future */
#define SL_ERR_INTERNAL 		2

typedef int slState;

struct slCamera {
	vec3 pos;
	vec3 lookDir;
	vec3 upDir;
	float fov;
	float aspect;
	float zNear;
	float zFar;
};

void slInit();
void slClose();
slState slGetState();
int slGetLastError();

void slGetCameraPos(vec3 pos);
void slGetCameraLookDir(vec3 lookDir);
void slGetCameraUpDir(vec3 upDir);
float slGetCameraFov();
float slGetCameraAspect();
float slGetCameraZNear();
float slGetCameraZFar();
void slSetCamera(vec3 pos, vec3 lookDir, vec3 upDir,
		float fov, float aspect, float zNear, float zFar);
void slSetCameraView(vec3 pos, vec3 lookDir);
void slSetCameraPos(vec3 pos);
void slSetCameraLookDir(vec3 lookDir);
void slSetCameraUpDir(vec3 upDir);
void slSetCameraFov(float fov);
void slSetCameraAspect(float aspect);
void slSetCameraZNear(float zNear);
void slSetCameraZFar(float zFar);
void slUpdateCamera();
void slUpdateCameraView();

void slSetBackgroundTex(unsigned int width, unsigned int height, char* data);
void slSetBackgroundTexSize(unsigned int width, unsigned int height);
void slSetBackgroundTexData(char* data);
void slUpdateBackgroundTex();
void slUpdateBackgroundTexData();

void slCreateEllipticCluster(unsigned int nParticles,
		float a, float b, vec3 n,
		float dr, float dz,
		int nBlurs, vec4* palette, float* blurSizes);
void slClearClusters();

void slRenderClassic();
void slUpdateGlobalTime(float t);

#endif /* SIMULATON_H */
