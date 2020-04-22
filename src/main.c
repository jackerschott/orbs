#include <math.h>
#include <stdio.h>

#include <GL/glx.h>
#include <X11/Xlib.h>

#include "simulation.h"

//GLuint prog;
//GLuint vertShader = 0;
//GLuint fragShader = 0;
//
//GLuint vertArr;
//
//void testInitGL(void);
//void testCloseGL(void);

//void testInitGL(void) {
//	glewInit();
//
//	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
//
//	const char *vertShaderSrc = \
//	    	"#version 320 es\n" \
//	    	"\n" \
//	    	"precision mediump float;\n" \
//	    	"\n" \
//	    	"in vec2 p;\n" \
//	    	"in vec3 color_vs;\n" \
//	    	"\n" \
//	    	"out vec4 color_fs;\n" \
//	    	"\n" \
//	    	"void main() {\n" \
//	    	" 	color_fs = vec4(color_vs.xyz, 1.0);\n" \
//	    	" 	gl_Position = vec4(p.xy, 0.0, 1.0);\n" \
//	    	"}";
//	const char *fragShaderSrc = \
//		"#version 320 es\n" \
//		"\n" \
//		"precision mediump float;\n" \
//		"\n" \
//		"in vec4 color_fs;\n" \
//		"\n" \
//		"out vec4 color_out;\n" \
//		"\n" \
//		"void main() {\n" \
//		" 	color_out = color_fs;\n" \
//		"}";
//
//	GLint success = 0;
//	prog = glCreateProgram();
//
//	vertShader = glCreateShader(GL_VERTEX_SHADER);
//	glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
//	glCompileShader(vertShader);
//	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
//	if (!success) {
//		GLint logLength;
//		glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
//
//		GLchar* log = (GLchar *)malloc(logLength);
//		glGetShaderInfoLog(vertShader, logLength, NULL, log);
//
//		fprintf(stderr, "Vert shader failed to compile:\n");
//		fprintf(stderr, "%s", log);
//
//		free(log);
//
//		testCloseGL();
//		return;
//	}
//	glAttachShader(prog, vertShader);
//
//	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
//	glCompileShader(fragShader);
//	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
//	if (!success) {
//		GLint logLength;
//		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
//
//		GLchar* log = (GLchar *)malloc(logLength);
//		glGetShaderInfoLog(fragShader, logLength, NULL, log);
//
//		fprintf(stderr, "Vert shader failed to compile:\n");
//		fprintf(stderr, "%s", log);
//
//		free(log);
//
//		testCloseGL();
//		return;
//	}
//	glAttachShader(prog, fragShader);
//
//	glLinkProgram(prog);
//	glGetProgramiv(prog, GL_LINK_STATUS, &success);
//	if (!success) {
//		GLint logLength;
//		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
//
//		GLchar* log = (GLchar *)malloc(logLength);
//		glGetProgramInfoLog(prog, logLength, NULL, log);
//
//		fprintf(stderr, "Program failed to link:\n");
//		fprintf(stderr, "%s", log);
//
//		free(log);
//
//		testCloseGL();
//		return;
//	}
//	glDetachShader(prog, vertShader);
//	glDetachShader(prog, fragShader);
//
//	vec2 pos[] = {
//		{  0.0f,  0.5f },
//		{ -0.5f, -0.5f },
//		{  0.5f, -0.5f },
//	};
//	vec3 cols[] = {
//		{ 1.0f, 0.0f, 0.0f },
//		{ 0.0f, 1.0f, 0.0f },
//		{ 0.0f, 0.0f, 1.0f },
//	};
//
//	GLuint posBuf, colBuf;
//	glGenVertexArrays(1, &vertArr);
//	glGenBuffers(1, &posBuf);
//	glGenBuffers(1, &colBuf);
//
//	glBindVertexArray(vertArr);
//	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, NULL);
//
//	glBindBuffer(GL_ARRAY_BUFFER, colBuf);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(cols), cols, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(1);
//	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, NULL);
//	glBindVertexArray(0);
//
//	//glBindVertexArray(vertArr);
//	//glBindBuffer(GL_ARRAY_BUFFER, posBuf);
//	//vec2 *posTest = (vec2 *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
//	//for (int i = 0; i < 3; ++i) {
//	//	printf("(%f %f)\n", posTest[i][0], posTest[i][1]);
//	//}
//	//glUnmapBuffer(GL_ARRAY_BUFFER);
//	//glBindVertexArray(0);
//}
//
//void testRenderGL(void) {
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	glUseProgram(prog);
//	glBindVertexArray(vertArr);
//	glDrawArrays(GL_TRIANGLES, 0, 3);
//	glBindVertexArray(0);
//	glUseProgram(0);
//}
//
//void testCloseGL(void) {
//	GLsizei nShaders;
//	GLuint attachedShaders[2];
//	glGetAttachedShaders(prog, 2, &nShaders, attachedShaders);
//	for (int i = 0; i < nShaders; ++i) glDetachShader(prog, attachedShaders[i]);
//	if (vertShader) glDeleteShader(vertShader);
//	if (fragShader) glDeleteShader(fragShader);
//	glDeleteProgram(prog);
//}


int main(int argc, char* argv[]) {
        Display* dpy;
        Window root, w;
        XVisualInfo* vi;
        Colormap cmap;
        GLXContext glc;
        XWindowAttributes gwa;
        XEvent e;

        /* TODO: Hardware independent attributes */
        GLint att[] = {
                GLX_RGBA,
                GLX_DEPTH_SIZE, 24,
                GLX_DOUBLEBUFFER,
                None,
        };
        XSetWindowAttributes swa;

        dpy = XOpenDisplay(NULL);
        if (dpy == NULL) {
                printf("Cannot connect to X server\n");
                return 1;
        }

        vi = glXChooseVisual(dpy, 0, att);
        if (vi == NULL) {
                printf("No appropriate visual could be found\n");
                return 1;
        }

        root = DefaultRootWindow(dpy);
        cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask = ExposureMask | KeyPressMask;

        w = XCreateWindow(dpy, root,
                        0, 0, 800, 800, 0,
                        vi->depth,
                        InputOutput,
                        vi->visual,
                        CWColormap | CWEventMask, &swa);

        XMapWindow(dpy, w);
        XStoreName(dpy, w, "Black Hole Simulation");

        glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
        glXMakeCurrent(dpy, w, glc);
        
        slInit();

        while (1) {
                XNextEvent(dpy, &e);

                if (e.type == Expose) {
                        XGetWindowAttributes(dpy, w, &gwa);
                        glViewport(0, 0, gwa.width, gwa.height);

                        vec3 pos = { 30.0f, 0.0f, 0.0f };
                        vec3 lookDir = { -pos[0], -pos[1], -pos[2] };
                        vec3 upDir = { 0.0f, 0.0f, 1.0f };
                        float fov = 60.0f * M_PI / 180.0f;
                        float aspect = ((float)gwa.width) / ((float)gwa.height);
                        float zNear = 0.1f;
                        float zFar = 100.0f;
                        slSetCamera(pos, lookDir, upDir,
					fov, aspect, zNear, zFar);
			slUpdateCamera();

			//slSetBackgroundTex()

                        unsigned int nParticles = 200000;
                        float a = 20.0f;
                        float b = 15.0f;
                        vec3 n = { 1.0f, -1.0f, 1.0f };
                        float dr = 1.0f;
                        float dz = 0.5f;
                        vec4 palette[] = { { 1.00f, 0.30f, 0.00f, 0.1f } };
                        float blurSizes[] = { 1.00f };
                        slCreateEllipticCluster(nParticles, a, b, n,
					dr, dz, 1, palette, blurSizes);

			slRenderClassic();

                        glXSwapBuffers(dpy, w);
                }
                else if (e.type == KeyPress) {
                        glXMakeCurrent(dpy, None, NULL);

			slClose();

                        glXDestroyContext(dpy, glc);
                        XDestroyWindow(dpy, w);
                        XCloseDisplay(dpy);
                        return 0;
                }
        }
}
