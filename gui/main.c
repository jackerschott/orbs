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
#define SCALEFAC_PER_WHEELINC 1.20f

Display *dpy;
XVisualInfo *vi;
Window win_main;
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
int viewport_update_completed = 1;

void *viewport_update(void* args)
{
	glXMakeCurrent(dpy, win_main, glc);
	vis_update();
	glXSwapBuffers(dpy, win_main);

	glXMakeCurrent(dpy, None, NULL);
	pthread_mutex_lock(&mutex);
	viewport_update_completed = 1;
	pthread_mutex_unlock(&mutex);
	return NULL;
}
int viewport_update_running()
{
	pthread_mutex_lock(&mutex);
	int retval = !viewport_update_completed;
	pthread_mutex_unlock(&mutex);

	if (!retval)
		glXMakeCurrent(dpy, win_main, glc);
	return retval;
}
void viewport_update_start()
{
	glXMakeCurrent(dpy, None, NULL);
	viewport_update_completed = 0;
	pthread_create(&render_thread, NULL, viewport_update, NULL);
}
void viewport_update_finish()
{
	pthread_join(render_thread, NULL);
	glXMakeCurrent(dpy, win_main, glc);
}

void on_button1press(XButtonEvent* e)
{
	vis_pin_observer((float)e->x / DOTS_PER_TURN,
			 (float)e->y / DOTS_PER_TURN);
}
void on_button1release(XButtonEvent* e)
{
	viewport_update_finish();
}
void on_motion(XMotionEvent* e)
{
	if (viewport_update_running())
	 	return;

	vis_move_pinned_observer((float)e->x / DOTS_PER_TURN,
				 (float)e->y / DOTS_PER_TURN);
	viewport_update_start();
}

void on_button4press(XButtonEvent* e)
{
	if (viewport_update_running()) {
		return;
	}

	vis_scale_observer(1.0f / SCALEFAC_PER_WHEELINC);
	vis_update();
	viewport_update_start();
}
void on_button5press(XButtonEvent* e)
{
	if (viewport_update_running())
		return;

	vis_scale_observer(SCALEFAC_PER_WHEELINC);
	vis_update();
	viewport_update_start();
}

void on_keypress(XKeyEvent* e)
{
	char c;
	KeySym ksym;
	XLookupString(e, &c, 2, &ksym, NULL);

	if (c == 'q') {
		quit = 1;
	}
	else if (c == 'l') {
		glXMakeCurrent(dpy, win_main, glc);
		struct Observer obs;
		orbs_get_observer(&obs);
		for (int i = 0; i < 100; ++i) {
			obs.pos[1] = (i / 99.0f) * 10.0f;
			orbs_set_observer(&obs);
			orbs_update_observer_vectors();

			viewport_update_start();
			viewport_update_finish();
		}
		glXMakeCurrent(dpy, None, NULL);
	}
}

void setup(Display *dpy, XVisualInfo* vi, Window* winMain)
{
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
			0, 0, 1920, 1080, 0,
			vi->depth,
			InputOutput,
			vi->visual,
			CWColormap | CWEventMask, &swa);

	XMapWindow(dpy, w);
	XStoreName(dpy, w, "orbs");

	*winMain = w;
}

void cleanup()
{
	XDestroyWindow(dpy, win_main);
	XCloseDisplay(dpy);
}

int loop_main()
{
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		printf("mutex init has failed");
		return 1;
	}

	XEvent e;
	while (!quit) {
		//pthread_mutex_lock(&mutex);
		XNextEvent(dpy, &e);
		//pthread_mutex_unlock(&mutex);

		if (e.type == Expose) {
			XGetWindowAttributes(dpy, e.xany.window, &gwa);
			glViewport(0, 0, gwa.width, gwa.height);

			vis_set_aspect((float)gwa.width / gwa.height);

			vis_update();
			viewport_update_start();
			viewport_update_finish();
		} else if (e.type == ButtonPress && e.xbutton.button == 1) {
			on_button1press(&e.xbutton);
		} else if (e.type == MotionNotify) {
			on_motion(&e.xmotion);
		} else if (e.type == ButtonRelease && e.xbutton.button == 1) {
			on_button1release(&e.xbutton);
		} else if (e.type == ButtonPress && e.xbutton.button == 4) {
			on_button4press(&e.xbutton);
		} else if (e.type == ButtonPress && e.xbutton.button == 5) {
			on_button5press(&e.xbutton);
		} else if (e.type == KeyPress) {
			on_keypress(&e.xkey);
		}
	}
	
	pthread_mutex_destroy(&mutex);

	return 0;
}

int main(int argc, char* argv[])
{
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

	setup(dpy, vi, &win_main);

	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win_main, glc);
	vis_setup();
	
	int retval = loop_main();

	vis_cleanup();
	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);

	cleanup();
	return retval;
}
