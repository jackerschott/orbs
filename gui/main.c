#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>

#include "orbs.h"
#include "visual.h"

#define DOTS_PER_TURN 2000

Display *dpy;
XVisualInfo *vi;
Window winMain;
GLXContext glc;
XWindowAttributes gwa;
int screen;
int quit = 0;

/* TODO: Hardware independent attributes */
GLint att[] = {
	GLX_RGBA,
	GLX_DEPTH_SIZE, 24,
	GLX_DOUBLEBUFFER,
	None,
};

pthread_t render_thread = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int visual_update_completed = 1;

void *visual_update(void* args) {
	glXMakeCurrent(dpy, winMain, glc);
	vis_update();
	glXSwapBuffers(dpy, winMain);

	pthread_mutex_lock(&mutex);
	visual_update_completed = 1;
	pthread_mutex_unlock(&mutex);

	return NULL;
}

void on_buttonpress(XButtonEvent* e) {
	vis_pin_observer((float)e->x / DOTS_PER_TURN,
			 (float)e->y / DOTS_PER_TURN);
}
void on_buttonrelease(XButtonEvent* e) {
	pthread_join(render_thread, NULL);
	glXSwapBuffers(dpy, winMain);
}
void on_motion(XMotionEvent* e) {

	pthread_mutex_lock(&mutex);
	int start_new_update = visual_update_completed;
	pthread_mutex_unlock(&mutex);

	if (start_new_update) {
		vis_move_pinned_observer((float)e->x / DOTS_PER_TURN,
					 (float)e->y / DOTS_PER_TURN);

		visual_update_completed = 0;
		pthread_create(&render_thread, NULL, visual_update, NULL);
	}
}
void on_keypress(XKeyEvent* e) {
	char c;
	KeySym ksym;
	XLookupString(e, &c, 2, &ksym, NULL);

	if (c == 'q') {
		quit = 1;
	}
}

void setup(Display *dpy, XVisualInfo* vi, Window* winMain) {
	Window root, w;
	Colormap cmap;
	XSetWindowAttributes swa;

	root = DefaultRootWindow(dpy);
	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.colormap = cmap;
	swa.event_mask = ExposureMask |
		KeyPressMask |
		ButtonPressMask |
		ButtonReleaseMask |
		Button1MotionMask;

	w = XCreateWindow(dpy, root,
			0, 0, 800, 800, 0,
			vi->depth,
			InputOutput,
			vi->visual,
			CWColormap | CWEventMask, &swa);

	XMapWindow(dpy, w);
	XStoreName(dpy, w, "orbs");

	*winMain = w;
}

void cleanup() {
	XDestroyWindow(dpy, winMain);
	XCloseDisplay(dpy);
}

int main(int argc, char* argv[]) {

	XInitThreads();
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		printf("Cannot connect to X server\n");
		return 1;
	}

	screen = XDefaultScreen(dpy);
	vi = glXChooseVisual(dpy, screen, att);
	if (vi == NULL) {
		printf("No appropriate visual could be found\n");
		return 1;
	}

	setup(dpy, vi, &winMain);
	
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, winMain, glc);
	vis_setup();
	
	XEvent e;
	while (!quit) {
		XNextEvent(dpy, &e);

		if (e.type == Expose) {
			XGetWindowAttributes(dpy, e.xany.window, &gwa);
			glViewport(0, 0, gwa.width, gwa.height);

			vis_set_aspect((float)gwa.width / gwa.height);
			vis_update();

			glXSwapBuffers(dpy, winMain);
		}
		else if (e.type == ButtonPress) {
			on_buttonpress(&e.xbutton);
		}
		else if (e.type == MotionNotify) {
			on_motion(&e.xmotion);
		}
		else if (e.type == ButtonRelease) {
			on_buttonrelease(&e.xbutton);
		}
		else if (e.type == KeyPress) {
			on_keypress(&e.xkey);
		}
	}

	vis_cleanup();
	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);

	cleanup();
	return 0;
}
