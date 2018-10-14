#ifndef GLWRAP_HPP
#define GLWRAP_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <string>

#include "fileman.hpp"
#include "res.hpp"

#define NUM_VERTEX_BUFFERS 4

namespace gl {

  union vertexArrayBuffers {
    struct {
      GLuint positions;
      GLuint texCoords;
      GLuint normals;
      GLuint indices;
    };
    GLuint buffers[NUM_VERTEX_BUFFERS];
  };

  bool createProgram(const char* vsSrcPath, const char* fsSrcPath, const char** inputNames,
    GLuint* prog, GLuint* vs, GLuint* fs, std::string* errLog = nullptr);

  bool checkForShaderErr(GLuint shader, GLenum errType);
  std::string getShaderInfoLog(GLuint shader);
  bool checkForProgramErr(GLuint program, GLenum errType);
  std::string getProgramInfoLog(GLuint program);

  std::string getErrMsg(GLenum err);
}

#endif
