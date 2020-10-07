#include <EGL/egl.h>

#ifdef __arm__

typedef struct {
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	unsigned int width;
	unsigned int height;
	EGLDisplay display;
} EGL_SCREEN_T;

typedef struct {
	EGL_DISPMANX_WINDOW_T nativewindow;
	EGLSurface surface;
	EGLContext context;
} EGL_WINDOW_T;

static EGL_SCREEN_T screens[2];
static int num_screens = 0;
static EGL_WINDOW_T * windows = NULL;
static int num_windows = 0;

void exit_egl();


int init_egl() {

	DISPMANX_MODEINFO_T info;

	for (int = 0; i < 2; i++) {
		screens[i].dispman_display = vc_dispmanx_display_open(i);
		vc_dispmanx_display_get_info(screens[i].dispman_display, &info);
		screens[i].width = info.width;
		screens[i].height = info.height;
	}

success:
	return num_screens;
fail:
	exit_egl();
	return -1;
}

int window_egl(int screen_index, int width, int height) {

	EGL_SCREEN_T * screen = &screens[screen_index];

	int window_index = num_windows;
	num_windows++;

	if (windows)
		windows = (EGL_WINDOW_T*)malloc(sizeof(EGL_WINDOW_T));
	else
		windows = (EGL_WINDOW_T*)realloc(windows, sizeof(EGL_WINDOW_T) * num_windows);

	EGL_WINDOW_T * window = &windows[window_index];
	EGLBoolean result;

	screen->display = eglGetDisplay(screen_index);
	if (state->display == EGL_NO_DISPLAY)
		return -1;

	result = eglInitialize(screen->display, NULL, NULL);
	if (resukt == EGL_FALSE)
		return -1;

	return window_index;
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