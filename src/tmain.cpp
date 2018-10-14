#define _USE_MATH_DEFINES

#include <chrono>
#include <cmath>
#include <fstream>
#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h>
#endif
#ifdef __unix__
#include <GL/glxew.h>
#endif
#include <iostream>
#include <SDL2/SDL.h>

#include "glWrapper/glwrap.hpp"

#include "render.hpp"
#include "res.hpp"
#include "tmeas.hpp"

// Entry point for testing purposes
int tmain(int argc, char** argv) {
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_Window* windowMain = SDL_CreateWindow("OpenGL Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800 * 16 / 9, 800, SDL_WINDOW_OPENGL);
  SDL_GLContext contextMain = SDL_GL_CreateContext(windowMain);

  float rs = 1.0f;
  const uint nParticles = 10000;

  camera camera;
  camera.pos = { 30.0f, 0.0f, 0.0f };
  camera.lookDir = -camera.pos;
  camera.upDir = { 0.0f, 0.0f, 1.0f };
  camera.fov = glm::radians(60.0f);
  camera.aspect = 16.0f / 9.0f;
  camera.zNear = 0.1f;
  camera.zFar = 100.0f;

  colorBlur colorPalette[3] = {
    { { 1.0f, 0.3f, 0.0f, 0.3f }, 1.0f },
  };

  int bgWidth;
  int bgHeight;
  int bgBpp;
  std::ifstream ifs("E:/tmp/sphere_map", std::ios::binary);
  ifs.read(reinterpret_cast<char*>(&bgWidth), sizeof(int));
  ifs.read(reinterpret_cast<char*>(&bgHeight), sizeof(int));
  ifs.read(reinterpret_cast<char*>(&bgBpp), sizeof(int));

  uint sBgImageData = bgWidth * bgHeight * bgBpp / 8;
  byte* bgImageData = new byte[sBgImageData];
  ifs.read(reinterpret_cast<char*>(bgImageData), sBgImageData);


  cl_context_properties platform = (cl_context_properties)cl::Platform::getDefault()();
#ifdef _WIN32
  cl_context_properties contextProps[] = {
    CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
    CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
    CL_CONTEXT_PLATFORM, platform,
    0
  };
#endif
#ifdef __unix__
  cl_context_properties contextProps[] = {
    CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
    CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDrawable(),
    CL_CONTEXT_PLATFORM, platform(),
    0
  };
#endif
  cl::Device device = cl::Device::getDefault();
  cl::Context context(device, contextProps);

  render::init(device, context, rs);
  render::setBackgroundTex(sBgImageData, bgImageData, bgWidth, bgHeight, bgBpp);
  render::createParticleRing(nParticles, 10.0f * rs, { 1.0f, -1.0f, 1.0f }, 1.0f * rs, 0.1f, 0.1f,
    sizeof(colorPalette) / sizeof(colorBlur), colorPalette);
  render::setObserverCamera(camera);


  initTimeMeas(true, 2.0);

  bool isClosed = false;
  bool isFirst = true;
  std::vector<double> renderTime;
  std::chrono::high_resolution_clock::time_point t0;
  std::chrono::high_resolution_clock::time_point t1;
  t0 = std::chrono::high_resolution_clock::now();
  while (!isClosed) {
    t1 = std::chrono::high_resolution_clock::now();
    double t = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() / 1.0e9;

    camera.pos = {
      30.0f * sin(M_PI_2 + sin(2.0 * M_PI * t / 30.0)) * cos(2.0 * M_PI * t / 60.0),
      30.0f * sin(M_PI_2 + sin(2.0 * M_PI * t / 30.0)) * sin(2.0 * M_PI * t / 60.0),
      30.0f * cos(M_PI_2 + sin(2.0 * M_PI * t / 30.0))
    };
    camera.lookDir = -camera.pos;
    render::moveObserverCamera(camera.pos, camera.lookDir, camera.upDir);

    setTimeMeasPoint();
    render::renderClassic();
    setTimeMeasPoint();

    if (evalLap(&renderTime)) {
      std::cout << "Render time: " << renderTime[0] << " s," << "\t\t"
        << "Frame rate: " << 1.0 / renderTime[0] << " fps" << std::endl;
      renderTime.clear();
    }

    SDL_GL_SwapWindow(windowMain);
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      isClosed = e.type == SDL_QUIT;
    }
  }

  std::vector<double> meanRenderTime;
  std::vector<double> stdDevMeanRenderTime;
  evalTimeMeas(&meanRenderTime, &stdDevMeanRenderTime);
  std::cout << std::endl;
  std::cout << "Render time: " << meanRenderTime[0] << " +- "
    << stdDevMeanRenderTime[0] << " s," << "\t\t"
    << "Frame rate: " << 1.0 / meanRenderTime[0] << " +- "
    << stdDevMeanRenderTime[0] / (meanRenderTime[0] * meanRenderTime[0]) << " fps" << std::endl;

	SDL_GL_DeleteContext(contextMain);
	SDL_DestroyWindow(windowMain);
	SDL_Quit();

	render::close();
	std::cin.get();
	return 0;
}
