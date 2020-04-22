#ifndef SC_H
#define SC_H

#include <GL/glew.h>
#include <GL/gl.h>

#define SC_ERR_NONE 		0
#define SC_ERR_SHADER_COMPILE 	1
#define SC_ERR_PROG_LINK 	2
#define SC_ERR_PROG_VALIDATE 	3

#ifndef SC_MAX_SHADERS_PER_PROG
#define SC_MAX_SHADERS_PER_PROG 8
#endif

GLuint scCompileProgram(int nShaders, GLuint shaderTypes[],
		const GLchar *const *shadersrcs);
int scGetCompileStatus();
int scGetErrorShaderIndex();
void scGetErrorLogLength(GLint* length);
void scGetErrorLog(GLint maxLength, GLint* length, GLchar* log);
void scFreeShaders(GLuint prog);
void scFreeProgram(GLuint prog);

#endif
