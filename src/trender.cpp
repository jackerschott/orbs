#include <tmain.hpp>
#include <GL/gl.h>
#include <CL/cl.hpp>

void trender(cl::Device clDevice, cl::Context clContext) {
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  initRenderSample(clDevice, clContext);
}
