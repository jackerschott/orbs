#include <sstream>

#include "gl.hpp"

namespace gl {
  const char* shaderTypeStr(GLenum type);

  program::program() {
    id = glCreateProgram();
  }

  program::~program() {
    for (int i = 0; i < (int)shaders.size(); ++i) {
      glDetachShader(id, shaders[i].id);
    }
    glDeleteProgram(id);
  }

  void program::attachShader(shader s) {
    glAttachShader(id, s.id);
    shaders.push_back(s);
  }

  bool program::build(std::string* log) {
    bool success = true;
    std::vector<std::string> logs;

    success &= compileShaders(&logs);

    logs.resize(logs.size() + 2);
    success &= link(logs.data() + logs.size() - 2);
    success &= validate(logs.data() + logs.size() - 1);

    std::ostringstream log_;
    if (log != NULL) {
      for (int i = 0; i < (int)shaders.size(); ++i) {
        if (logs[i].length() > 0) {
          std::string title = shaders[i].fileName;
          std::string underscore;
          for (int i = 0; i < (int)title.length(); ++i) {
            underscore += "─";
          }
          log_ << title << std::endl;
          log_ << underscore << std::endl;
          log_ << logs[i] << std::endl;
          log_ << std::endl;
        }
      }
      if (logs[logs.size() - 2].length() > 0) {
        log_ << "Linkage" << std::endl;
        log_ << "───────" << std::endl;
        log_ << logs[logs.size() - 2] << std::endl;
        log_ << std::endl;
      }

      if (logs[logs.size() - 1].length() > 0) {
        log_ << "Validation" << std::endl;
        log_ << "──────────" << std::endl;
        log_ << logs[logs.size() - 1] << std::endl;
        log_ << std::endl;
      }
    }
    *log = log_.str();
    return success;
  }

  bool program::compileShaders(std::vector<std::string>* logs) {
    bool success = true;
    std::vector<std::string> logs_(shaders.size());
    for (int i = 0; i < (int)shaders.size(); ++i) {
      success &= shaders[i].compile(logs_.data() + i);
    }
    if (logs != NULL)
      *logs = logs_;
    return success;
  }
  bool program::link(std::string* log) {
    int success;
    char log_[0x400];

    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(id, sizeof(log_), NULL, log_);
      if (log != NULL)
        *log = std::string(log_);
    }
    return success;
  }
  bool program::validate(std::string* log) {
    int success;
    char log_[0x400];

    glValidateProgram(id);
    glGetProgramiv(id, GL_VALIDATE_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(id, sizeof(log_), NULL, log_);
      if (log != NULL)
        *log = std::string(log_);
    }
    return success;
  }

  const char* shaderTypeStr(GLenum type) {
    switch (type)
    {
    case GL_VERTEX_SHADER:
      return "GL_VERTEX_SHADER";
    case GL_FRAGMENT_SHADER:
      return "GL_FRAGMENT_SHADER";
    case GL_COMPUTE_SHADER:
      return "GL_COMPUTE_SHADER";
    case GL_GEOMETRY_SHADER:
      return "GL_GEOMETRY_SHADER";
    default:
      return "UNKNOWN_SHADER_TYPE";
    }
  }
}
