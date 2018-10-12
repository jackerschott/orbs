#define _USE_MATH_DEFINES

#include <chrono>
#include <cmath>
#include <fstream>
#include <GL/glew.h>
#include <iostream>
#include <SDL2/SDL.h>

#include "glWrapper/glwrap.hpp"

#include "render.hpp"
#include "res.hpp"

void initRenderSample(cl::Device device, cl::Context context);

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

  cl::Platform platform = cl::Platform::getDefault();
  cl::Device device = cl::Device::getDefault();
  cl_context_properties contextProps[] = {
    CL_GL_CONTEXT_KHR,
    (cl_context_properties)contextMain,
    CL_WGL_HDC_KHR,
    (cl_context_properties)contextMain,
    CL_CONTEXT_PLATFORM,
    (cl_context_properties)platform(),
    0
  };
  cl::Context context(device, contextProps);
  initRenderSample(device, context);

#define TIME_BUF_SIZE 100
#define REF_TIME_MAX 2000
  int t = 0;
  std::vector<double> rTimes;
  double rTimeBuf[TIME_BUF_SIZE];
  memset(rTimeBuf, 0, sizeof(rTimeBuf));

	bool isClosed = false;
  bool isFirst = true;
   std::chrono::high_resolution_clock::time_point tr = std::chrono::high_resolution_clock::now();
	while (!isClosed) {

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    render::render();
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    double renderTime = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / 1.0e9;
    int refTime = (int)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - tr).count();
    rTimes.push_back(renderTime);
    rTimeBuf[t] = renderTime;
    t = (t + 1) % TIME_BUF_SIZE;
    if (refTime > REF_TIME_MAX) {
      tr = std::chrono::high_resolution_clock::now();
      double meanTime = 0.0;
      for (uint i = 0; i < TIME_BUF_SIZE; i++) {
        meanTime += rTimeBuf[i];
      }
      memset(rTimeBuf, 0, sizeof(rTimeBuf));
      meanTime /= TIME_BUF_SIZE;
      std::cout << "Render Time: " << meanTime << " s" << ",\t\t";
      std::cout << "Frame Rate: " << 1.0 / meanTime << " fps" << std::endl;
    }

		SDL_GL_SwapWindow(windowMain);
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			isClosed = e.type == SDL_QUIT;
		}
	}

	SDL_GL_DeleteContext(contextMain);
	SDL_DestroyWindow(windowMain);
	SDL_Quit();

	render::close();

  double meanRenderTime = 0.0;
  double stdDevMeanRenderTime = 0.0;
  for (uint i = 0; i < rTimes.size(); i++) {
    meanRenderTime += rTimes[i];
  }
  meanRenderTime /= rTimes.size();
  for (uint i = 0; i < rTimes.size(); i++) {
    stdDevMeanRenderTime += (rTimes[i] - stdDevMeanRenderTime) * (rTimes[i] - stdDevMeanRenderTime);
  }
  stdDevMeanRenderTime = sqrt(stdDevMeanRenderTime / (rTimes.size() * (rTimes.size() - 1)));

  std::cout << std::endl;
  std::cout << "Render time mean: (" << meanRenderTime << " +- " << stdDevMeanRenderTime << ") s" << std::endl;
  std::cout << "Frame rate mean: (" << 1.0 / meanRenderTime << " +- " << stdDevMeanRenderTime / (meanRenderTime * meanRenderTime) << ") fps" << std::endl;

	std::cin.get();
	return 0;
}

void initRenderSample(cl::Device device, cl::Context context) {
	float rs = 1.0f;
	float rg = 10.0f * rs;

	const uint nParticles = 100000;

	camera camera;
	camera.pos = { 30.0f, 0.0f, 0.0f };
	camera.lookDir = { -1.0f, 0.0f, 0.0f };
	camera.upDir = { 0.0f, 0.0f, 1.0f };
	camera.fov = 60.0f;
  camera.aspect = 16.0f / 9.0f;
  camera.zNear = 0.1f;
  camera.zFar = 100.0f;

  colorBlur colorPalette[3] = {
    { { 1.0f, 0.3f, 0.0f, 0.3f }, 1.0f },
  };

  //int bgWidth;
  //int bgHeight;
  //int bgBpp;

  //std::ifstream ifs(GIT_FOLDER_PATH "res/sphere_map", std::ios::binary);
  //ifs.read(reinterpret_cast<char*>(&bgWidth), sizeof(int));
  //ifs.read(reinterpret_cast<char*>(&bgHeight), sizeof(int));
  //ifs.read(reinterpret_cast<char*>(&bgBpp), sizeof(int));

  //uint sBgImageData = bgWidth * bgHeight * bgBpp / 8;
  //byte* bgImageData = new byte[sBgImageData];
  //ifs.read(reinterpret_cast<char*>(bgImageData), sBgImageData);

	render::init(rs, rg);
  render::initHardwAcc(device, context);
  // render::setBackground(sBgImageData, bgImageData, bgWidth, bgHeight, bgBpp);
	render::createParticleRing(nParticles, 10.0f * rs, { 1.0f, -1.0f, 1.0f }, 1.0f * rs, 0.1f, 0.1f,
    sizeof(colorPalette) / sizeof(colorBlur), colorPalette);
	render::setObserver(camera);
}
