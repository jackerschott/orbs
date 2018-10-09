#include "glWrapper/glwrap.hpp"

namespace gl {

#define N_SRCS 1
	GLuint createShader(std::string& src, uint shaderType);

	program::program(const char* vertexShaderSrcPath, const char* fragmentShaderSrcPath, std::string* err) {
		std::string vertexShaderSrc;
		std::string fragmentShaderSrc;
		std::string shaderLoadErrMsg;
		hasLoadedShader = true;
		
		prog = glCreateProgram();
		if (loadFile(vertexShaderSrcPath, vertexShaderSrc, &shaderLoadErrMsg)) {
			shaders[SHADER_VERTEX] = createShader(vertexShaderSrc, GL_VERTEX_SHADER);
		}
		else {
			if (err != nullptr)
				*err = shaderLoadErrMsg;
			hasLoadedShader = false;
			return;
		}
		if (loadFile(vertexShaderSrcPath, vertexShaderSrc, &shaderLoadErrMsg)) {
			shaders[SHADER_FRAGMENT] = createShader(fragmentShaderSrc, GL_FRAGMENT_SHADER);
		}
		else {
			if (err != nullptr)
				*err = shaderLoadErrMsg;
			hasLoadedShader = false;
			return;
		}
		
		glAttachShader(prog, shaders[SHADER_VERTEX]);
		glAttachShader(prog, shaders[SHADER_FRAGMENT]);

		glBindAttribLocation(prog, 0, "position");
		glBindAttribLocation(prog, 1, "texCoord");
		glBindAttribLocation(prog, 2, "normal");

		glLinkProgram(prog);
		glValidateProgram(prog);
	}

	program::~program() {
		if (hasLoadedShader) {
			glDetachShader(prog, shaders[SHADER_VERTEX]);
			glDetachShader(prog, shaders[SHADER_FRAGMENT]);
			glDeleteShader(shaders[SHADER_VERTEX]);
			glDeleteShader(shaders[SHADER_FRAGMENT]);
		}
		glDeleteProgram(prog);
	}

	void program::bind() {
		glUseProgram(prog);
	}

	GLuint createShader(std::string& src, uint shaderType) {
		GLuint shader = glCreateShader(GL_VERTEX_SHADER);

		const char* srcStr = src.c_str();
		int srcLen = (int)src.length();
		glShaderSource(shader, N_SRCS, &srcStr, &srcLen);
		glCompileShader(shader);
		return shader;
	}
}