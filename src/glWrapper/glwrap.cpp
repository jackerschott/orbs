#include "glWrapper/glwrap.hpp"

#define LOG_BUFFER_SIZE 1024

namespace gl {
  bool checkForShaderErr(GLuint shader, GLenum errType) {
    GLint success;
    glGetShaderiv(shader, errType, &success);
    return !success;
  }
  std::string getShaderInfoLog(GLuint shader) {
    char log[LOG_BUFFER_SIZE];
    int logLen;
    glGetShaderInfoLog(shader, sizeof(log), &logLen, log);
    return std::string(log, logLen);
  }
  bool checkForProgramErr(GLuint program, GLenum errType) {
    GLint success;
    glGetProgramiv(program, errType, &success);
    return !success;
  }
  std::string getProgramInfoLog(GLuint program) {
    char log[LOG_BUFFER_SIZE];
    int logLen;
    glGetProgramInfoLog(program, sizeof(log), &logLen, log);
    return std::string(log, logLen);
  }

  std::string getErrMsg(GLenum err) {
    switch (err)
    {
    case GL_NO_ERROR: return "Operation finished successfully.";
    case GL_INVALID_ENUM: return "GLenum argument out of range.";
    case GL_INVALID_VALUE: return "Numeric argument out of range.";
    case GL_INVALID_OPERATION: return "Operation illegal in current state.";
    case GL_STACK_OVERFLOW: return "Function would cause a stack overflow.";
    case GL_STACK_UNDERFLOW: return "Function would cause a stack underflow.";
    case GL_OUT_OF_MEMORY: return "Not enough memory left to execute function.";
    default: return "Unknown OpenGL error.";
    }
  }
}