#include <EGL/egl.h>

#ifdef __arm__

typedef struct {
	int width;
	int height;
} EGL_SCREEN_T;

typedef struct {
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
} EGL_WINDOW_T;

static EGL_SCREEN_T * screens;
static int num_screens = 0;
static EGL_WINDOW_T * windows;
static int num_windows = 0;

void exit_egl();

int init_egl() {

success:
	return num_screens;
fail:
	exit_egl();
	return -1;
}

int window_egl(int screen_index, int width, int height) {

	EGL_SCREEN_T * screen = &screens[screen_index];

	return -1;
}

int loop_egl() {
	return 1;
}

void size_egl(int window_index, int * width, int * height) {
}

void redraw_egl(int window_index) {
	EGL_WINDOW_T * window = &windows[window_index];
	eglSwapBuffers(window->display, window->surface);
}

void exit_egl() {
}

#endif