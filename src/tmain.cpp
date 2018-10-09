#define _USE_MATH_DEFINES

#define ENCODER_PATH "E:/Benutzerdateien/Documents/Visual Studio 2017/Projects/PngEncoder/PngEncoder/bin/Release/PngEncoder.exe"
#define TMP_IMG_DATA_PATH "E:/tmp/imgbin"
#define IMG_PATH "E:/tmp/img.png"

#include <chrono>
#include <fstream>
#include <GL/glew.h>
#include <iostream>
#include <cmath>
#include <SDL.h>
#include <windows.h>

#include "glWrapper/glwrap.hpp"

#include "render.hpp"
#include "res.hpp"

#define VERTEX_SHADER_SRC_PATH ""
#define FRAGMENT_SHADER_SRC_PATH ""

void initRenderSample(bool hardwAcc = true);
int encodePng(const char* dataPath, const char* imgPath);

// Entry point for testing purposes
int tmain(int argc, char** argv) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Window* windowMain = SDL_CreateWindow("OpenGL Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920 / 2, 1080 / 2, SDL_WINDOW_OPENGL);
	SDL_GLContext contextMain = SDL_GL_CreateContext(windowMain);

	glewInit();

	gl::vertex vertices[] = {
		{ glm::vec3(-0.5, -0.5, 0.0) },
		{ glm::vec3(0.0, 0.5, 0.0) },
		{ glm::vec3(0.5, -0.5, 0.0) }
	};

	gl::mesh triangle(sizeof(vertices) / sizeof(gl::vertex), vertices);
	gl::program prog(VERTEX_SHADER_SRC_PATH, FRAGMENT_SHADER_SRC_PATH);

	bool isClosed = false;
	while (!isClosed) {

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		prog.bind();
		triangle.draw();

		SDL_GL_SwapWindow(windowMain);
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			isClosed = e.type == SDL_QUIT;
		}
	}

	//initRenderSample();
	//render::render();
	//while (render::isRendering()); 
	//byte* imageData = render::getImageData();

	//std::ofstream imgStream(TMP_IMG_DATA_PATH, std::ios::binary);
	//imgStream.write(reinterpret_cast<char*>(&render::pWidth), sizeof(uint));
	//imgStream.write(reinterpret_cast<char*>(&render::pHeight), sizeof(uint));
	//imgStream.write(reinterpret_cast<char*>((uint*)&render::bpp), sizeof(uint));
	//imgStream.write(reinterpret_cast<char*>(imageData), render::sPixelData);
	//imgStream.close();
	//encodePng(TMP_IMG_DATA_PATH, IMG_PATH);
	//std::remove(TMP_IMG_DATA_PATH);

	//SDL_GL_DeleteContext(contextMain);
	SDL_DestroyWindow(windowMain);
	SDL_Quit();

	//render::close();
	std::cin.get();
	return 0;
}

void initRenderSample(bool hardwAcc) {
	uint w = 1920;
	uint h = 1080;
	float rs = 1.0f;
	float rg = 10.0f * rs;

	const uint nParticles = 500;

	perspectiveCamera camera;
	camera.pos = { 30.0f, 0.0f, 0.0f };
	camera.lookDir = { -1.0f, 0.0f, 0.0f };
	camera.upDir = { 0.0f, 0.0f, 1.0f };
	camera.fov = 60.0f;

	probColor *colorPalette = new probColor[1];
	for (uint i = 0; i < 1; i++) {
		colorPalette[i] = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	render::init(rs, rg);
	render::initHardwAcc(cl::Platform::getDefault(), cl::Device::getDefault());
	render::createParticleRing(nParticles, 5.0f * rs, { 1.0f, -1.0f, 1.0f }, 0.1f * rs, 0.1f, 0.1f, 1, colorPalette);
	render::setCamera(camera);

	render::config(w, h, partRad0, hardwAcc);
}
int encodePng(const char* dataPath, const char* imgPath) {
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);

	std::string cmdLine = "\"" + std::string(ENCODER_PATH) + "\" " + std::string(dataPath) + " " + std::string(imgPath);
	CreateProcess(ENCODER_PATH, (char*)cmdLine.c_str(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}
