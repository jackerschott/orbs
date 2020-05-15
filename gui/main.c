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
int viewport_update_completed = 1;

void *viewport_update(void* args)
{
	glXMakeCurrent(dpy, winMain, glc);
	vis_update();
	glXSwapBuffers(dpy, winMain);

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
	return retval;
}
void viewport_update_start()
{
	viewport_update_completed = 0;
	pthread_create(&render_thread, NULL, viewport_update, NULL);
}
void viewport_update_finish()
{
	pthread_join(render_thread, NULL);
}

void on_button1press(XButtonEvent* e)
{
	vis_pin_observer((float)e->x / DOTS_PER_TURN,
			 (float)e->y / DOTS_PER_TURN);
}
void on_button1release(XButtonEvent* e)
{
	//viewport_update_finish();
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
	if (viewport_update_running())
		return;

	vis_scale_observer(1.0f / SCALEFAC_PER_WHEELINC);
	viewport_update_start();
}
void on_button5press(XButtonEvent* e)
{
	if (viewport_update_running())
		return;

	vis_scale_observer(SCALEFAC_PER_WHEELINC);
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
	XDestroyWindow(dpy, winMain);
	XCloseDisplay(dpy);
}

int loop_main()
{
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
		else if (e.type == ButtonPress && e.xbutton.button == 1) {
			on_button1press(&e.xbutton);
		}
		else if (e.type == MotionNotify) {
			on_motion(&e.xmotion);
		}
		else if (e.type == ButtonRelease && e.xbutton.button == 1) {
			on_button1release(&e.xbutton);
		}
		else if (e.type == ButtonPress && e.xbutton.button == 4) {
			on_button4press(&e.xbutton);
		}
		else if (e.type == ButtonPress && e.xbutton.button == 5) {
			on_button5press(&e.xbutton);
		}
		else if (e.type == KeyPress) {
			on_keypress(&e.xkey);
		}
	}
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

	setup(dpy, vi, &winMain);
	
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, winMain, glc);
	vis_setup();
	
	int retval = loop_main();

	vis_cleanup();
	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);

	cleanup();
	return retval;
}
