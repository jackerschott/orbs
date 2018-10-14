#include "glWrapper/glwrap.hpp"

#define LOG_BUFFER_SIZE 1024

namespace gl {
  bool createProgram(const char* vsSrcPath, const char* fsSrcPath, const char** inputNames,
    GLuint* prog, GLuint* vs, GLuint* fs, std::string* errLog) {
    bool success = true;
    *errLog = std::string();
    std::string source;
    const char* src;
    int len;

    *vs = glCreateShader(GL_VERTEX_SHADER);
    loadFile(vsSrcPath, source);
    src = source.c_str();
    len = (int)source.length();
    glShaderSource(*vs, 1, &src, &len);

    *fs = glCreateShader(GL_FRAGMENT_SHADER);
    loadFile(fsSrcPath, source);
    src = source.c_str();
    len = (int)source.length();
    glShaderSource(*fs, 1, &src, &len);

    *prog = glCreateProgram();
    glCompileShader(*vs);
    if (gl::checkForShaderErr(*vs, GL_COMPILE_STATUS)) {
      std::string log = gl::getShaderInfoLog(*vs);
      *errLog += log + (log.length() > 0 ? "\n" : "");
      success = false;
    }
    glAttachShader(*prog, *vs);

    glCompileShader(*fs);
    if (gl::checkForShaderErr(*fs, GL_COMPILE_STATUS)) {
      std::string log = gl::getShaderInfoLog(*fs);
      *errLog += log + (log.length() > 0 ? "\n" : "");
      success = false;
    }
    glAttachShader(*prog, *fs);

    for (uint i = 0; i < NUM_PT_SHADER_ATTR; i++) {
      glBindAttribLocation(*prog, i, inputNames[i]);
    }
    glLinkProgram(*prog);
    if (gl::checkForProgramErr(*prog, GL_LINK_STATUS)) {
      std::string log = gl::getProgramInfoLog(*prog);
      *errLog += log + (log.length() > 0 ? "\n" : "");
      success = false;
    }
    glValidateProgram(*prog);
    if (gl::checkForProgramErr(*prog, GL_VALIDATE_STATUS)) {
      std::string log = gl::getProgramInfoLog(*prog);
      *errLog += log + (log.length() > 0 ? "\n" : "");
      success = false;
    }
    return success;
  }

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
