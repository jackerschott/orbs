#ifndef GLWRAP_HPP
#define GLWRAP_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "fileman.hpp"
#include "res.hpp"

namespace gl {
	class program;
	class mesh;
	struct vertex;

	enum shaderTypes {
		SHADER_VERTEX,
		SHADER_FRAGMENT
	};

	enum vertProps {
		VP_POSITION,
		VP_TEX_COORD,
		VP_NORMAL,
		VP_INDEX
	};

#define N_SHADERS 2
	class program {
	public:
		program(const char* vertexShader, const char* fragmentShader);
		virtual ~program();

		void bind();

	private:
		GLuint prog;
		GLuint shaders[N_SHADERS];
	};

#define N_MESH_ARRAYS 4
	class mesh {
	public:
		mesh(uint nVerts, vertex* verts);
		virtual ~mesh();

		void draw();

	private:
		uint nVerts;
		vertex* verts;

		GLuint vertexArrays;
		GLuint vertexArrayBuffers[N_MESH_ARRAYS];
	};

	struct vertex {
		glm::vec3 pos;
	};

	bool getLastShaderError(GLuint shader, GLuint flag, std::string& errMsg);
	bool getLastProgramError(GLuint program, GLuint flag, std::string& errMsg);
}

#endif
