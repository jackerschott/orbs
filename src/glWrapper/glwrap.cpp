#include "glWrapper/glwrap.hpp"

#define LOG_BUFFER_SIZE 1024

DEF_STANDARD_TYPES

namespace gl {
  const char* getErrSource(GLenum source);
  const char* getErrType(GLenum type);
  const char* getErrSeverity(GLenum severity);

  bool createProgram(const char* vsSrcPath, const char* fsSrcPath, const char** inputNames,
    GLuint* prog, GLuint* vs, GLuint* fs, std::string* errLog) {
    bool success = true;
    *errLog = std::string();
    std::string source;
    const char* src;
    int len;

    *vs = glCreateShader(GL_VERTEX_SHADER);
    loadFile(vsSrcPath, &source);
    src = source.c_str();
    len = (int)source.length();
    glShaderSource(*vs, 1, &src, &len);

    *fs = glCreateShader(GL_FRAGMENT_SHADER);
    loadFile(fsSrcPath, &source);
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
