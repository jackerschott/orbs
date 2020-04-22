#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sc.h"

int errType;
GLuint errObj;
int errShaderIndex = -1;

GLuint scCompileProgram(int nShaders, GLuint shaderTypes[],
		const GLchar *const *shadersrcs) {
	GLint success = 0;

	GLuint prog = glCreateProgram();

	for (int i = 0; i < nShaders; ++i) {
		GLuint shader = glCreateShader(shaderTypes[i]);
		glShaderSource(shader, 1, &shadersrcs[i], NULL);

		glCompileShader(shader);
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			errType = SC_ERR_SHADER_COMPILE;
			errObj = shader;
			errShaderIndex = errShaderIndex;
			return prog;
		}

		glAttachShader(prog, shader);
	}

	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success) {
		errType = SC_ERR_PROG_LINK;
		errObj = prog;
		return prog;
	}

	glValidateProgram(prog);
	if (!success) {
		errType = SC_ERR_PROG_VALIDATE;
		errObj = prog;
		return prog;
	}

	errType = SC_ERR_NONE;
	return prog;
}

int scGetCompileStatus() {
	return errType;
}
int scGetErrorShaderIndex() {
	return errShaderIndex;
}

void scGetErrorLogLength(GLint* length) {
	switch (errType) {
	case SC_ERR_SHADER_COMPILE:
		glGetShaderiv(errObj, GL_INFO_LOG_LENGTH, length);
		break;
	case SC_ERR_PROG_LINK:
	case SC_ERR_PROG_VALIDATE:
		glGetProgramiv(errObj, GL_INFO_LOG_LENGTH, length);
		break;
	}
}
void scGetErrorLog(GLint maxLength, GLint* length, GLchar* log) {
	switch (errType) {
	case SC_ERR_SHADER_COMPILE:
		glGetShaderInfoLog(errObj, maxLength, length, log);
		break;
	case SC_ERR_PROG_LINK:
	case SC_ERR_PROG_VALIDATE:
		glGetProgramInfoLog(errObj, maxLength, length, log);
		break;
	}
}

void scFreeShaders(GLuint prog) {
	/* TODO: Check if shaders need to be initialized as array */
	/* If no shaders are attached due to a compilation error,
	   this will do nothing */
	GLsizei nShaders;
	GLuint* shaders;
	glGetAttachedShaders(prog, SC_MAX_SHADERS_PER_PROG, &nShaders, shaders);
	for (int i = 0; i < nShaders; ++i) {
		glDetachShader(prog, shaders[i]);
		glDeleteShader(shaders[i]);
	}
	/* TODO: Check if shaders need to be freed (probably not) */
	//free(shaders);

	/* In case of compilation error, delete corresponding non attached shader */
	if (errType == SC_ERR_SHADER_COMPILE) {
		glDeleteShader(errObj);
	}
}
void scFreeProgram(GLuint prog) {
	glDeleteProgram(prog);
}

