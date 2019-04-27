#ifndef GLINC_HPP
#define GLINC_HPP

#include <GL/glew.h>

#include <map>
#include <vector>

namespace gl {

  class shader {
  public:
    GLuint id;
    GLenum type;
  private:
    std::string fileName;
    std::string src;
    std::map<std::string, std::string> incSrcs;

  public:
    shader(GLenum type);
    shader(GLenum type, const char* srcPath);
    virtual ~shader();

    void setIncludeSrc(const char* name, const char* srcPath);
    void setSrc(const char* srcPath);

  private:
    bool compile(std::string* log = NULL);
    std::string insertIncludes(std::string shaderSrc);

    friend class program;
  };

  class program {
  public:
    GLuint id;
  private:
    std::vector<shader> shaders;

  public:
    program();
    virtual ~program();

    void attachShader(shader s);
    bool build(std::string* log = NULL);

  private:
    bool compileShaders(std::vector<std::string>* logs = NULL);
    bool link(std::string* log = NULL);
    bool validate(std::string* log = NULL);
  };
}

#endif
