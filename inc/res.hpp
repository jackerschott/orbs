// Device: NVIDIA GeForce GTX 1080 Ti with OpenCL 1.2 CUDA
// OpenGL Version: 4.6.0 NVIDIA 397.44
// OpenGL Shading Language Version: 4.60 NVIDIA

#ifndef RES_HPP
#define RES_HPP

typedef unsigned char byte;
typedef unsigned int uint;
typedef size_t uslong;

// Colors
struct probColor {
  byte r;
  byte g;
  byte b;
  byte p;
};

// OpenGL
#ifdef __unix__
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
extern const char* glPtShaderInNames[];
extern const char* glPtShaderUniNames[];

#define BG_VERTEX_SHADER_SRC_PATH GIT_FOLDER_PATH "src/shaders/bg_vert.shader"
#define BG_FRAGMENT_SHADER_SRC_PATH GIT_FOLDER_PATH "src/shaders/bg_frag.shader"
#define NUM_BG_SHADER_ATTR 2
enum bgShaderIn {
  BG_POS,
  BG_TEX_COORD
};
enum bgShaderUniform {

};
extern const char* glBgShaderInNames[];

// OpenCL
#define RENDER_KERNEL_SRC_PATH GIT_FOLDER_PATH "src/kernels/render.cl"
#define NUM_KERNELS 1
enum kernels {
  KERNEL_GET_PT_POSITIONS,
  KERNEL_PROCESS_IMAGE
};
extern const char* kernelNames[];

namespace std {
  class invalid_operation : exception {
    virtual const char* what();
  };
}

#endif
