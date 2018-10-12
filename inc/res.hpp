#ifndef RES_HPP
#define RES_HPP

#include <GL/glew.h>
#include <string>

typedef unsigned char byte;
typedef unsigned int uint;
typedef size_t ulong;

// OpenGL
#ifdef __linux__
#define GIT_FOLDER_PATH "./"
#endif
#ifdef _WIN32
#define GIT_FOLDER_PATH "../../"
#endif

#define PT_VERTEX_SHADER_SRC_PATH GIT_FOLDER_PATH "src/shaders/pt_vert.shader"
#define PT_FRAGMENT_SHADER_SRC_PATH GIT_FOLDER_PATH "src/shaders/pt_frag.shader"
#define NUM_PT_SHADER_ATTR 2
#define NUM_PT_SHADER_UNIFORMS 1
enum ptShaderIn {
  PT_POS,
  PT_COLOR
};
enum ptShaderUniform {
  PT_VIEW_PROJ
};
extern const char* glPtShaderIns[];
extern const char* glPtShaderUniforms[];

#define BG_VERTEX_SHADER_SRC_PATH GIT_FOLDER_PATH "src/shaders/bg_vert.shader"
#define BG_FRAGMENT_SHADER_SRC_PATH GIT_FOLDER_PATH "src/shaders/bg_frag.shader"
#define NUM_BG_SHADER_ATTR 2
enum bgShaderIn {
  BG_POS,
  BG_TEX_COORD
};
enum bgShaderUniform {

};
extern const char* glBgShaderIns[];

// OpenCL
#define RENDER_KERNEL_SRC_PATH GIT_FOLDER_PATH "src/kernels/render.cl"
#define NUM_KERNELS 1
enum kernels {
  GET_PT_POSITIONS
};
extern const char* kernelNames[];

#endif
