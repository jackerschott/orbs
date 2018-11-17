#ifndef RES_HPP
#define RES_HPP

#ifdef __unix
#define DEF_STANDARD_TYPES \
  typedef unsigned char byte; \
  typedef long int64; \
  typedef unsigned long uint64;
#endif
#ifdef _WIN32
#ifdef _WIN64
#define DEF_STANDARD_TYPES \
  typedef unsigned char byte; \
  typedef unsigned int uint; \
  typedef long long int64; \
  typedef unsigned long long uint64;
#elif
#define DEF_STANDARD_TYPES \
  typedef unsigned char byte; \
  typedef unsigned int uint; \
  typedef long int64; \
  typedef unsigned long uint64;
#endif
#endif

#ifdef _DEBUG
#ifdef __unix__
#define INSTALL_PATH "/home/stephan/Projects/black_hole_simulation/"
#endif
#ifdef _WIN32
#define INSTALL_PATH "../../"
#endif
#else
#define INSTALL_PATH "./"
#endif

// OpenGL
#ifdef _DEBUG
#define SHADER_PATH INSTALL_PATH "src/shaders/"
#else
#define SHADER_PATH INSTALL_PATH "shaders/"
#endif

#define PT_VERTEX_SHADER_SRC_PATH SHADER_PATH "pt_vert.shader"
#define PT_FRAGMENT_SHADER_SRC_PATH SHADER_PATH "pt_frag.shader"
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

#define BG_VERTEX_SHADER_SRC_PATH SHADER_PATH "bg_vert.shader"
#define BG_FRAGMENT_SHADER_SRC_PATH SHADER_PATH "bg_frag.shader"
#define NUM_BG_SHADER_ATTR 2
enum bgShaderIn {
  BG_POS,
  BG_TEX_COORD
};
enum bgShaderUniform {};
extern const char* glBgShaderInNames[];

// OpenCL
#ifdef _DEBUG
#define KERNEL_PATH INSTALL_PATH "src/kernels/"
#else
#define KERNEL_PATH INSTALL_PATH "kernels/"
#endif

#define PTGEN_KERNEL_SRC_PATH KERNEL_PATH "ptgen.cl"
#define PTGEN_NUM_KERNELS 2
enum ptgenKernels {
  KERNEL_GET_ELLIPTIC_PT_DISTR,
  KERNEL_GET_PT_COLORS
};
extern const char* ptgenKernelNames[];

#define RNG_KERNEL_SRC_PATH KERNEL_PATH "rng.cl"
#define RNG_NUM_KERNELS 3
enum rngKernels {
  KERNEL_GEN_SAMPLES,
  KERNEL_GEN_FLOAT_SAMPLES,
  KERNEL_GEN_GAUSSIAN_SAMPLES
};
extern const char* rngKernelNames[];

#endif
