#include <stdlib.h>
#include <EGL/egl.h>
#if ENABLE_KMS
#include "kms.h"
#endif
#if ENABLE_DMX
#include "dmx.h"
#endif

typedef struct {
	EGLNativeDisplayType displayType;
#if ENABLE_KMS
	kms_output output;
#elif ENABLE_DMX
	dmx_screen output;
#endif
	EGLDisplay display;
	EGLConfig config;
	EGLint num_config;
} EGL_SCREEN_T;

typedef struct {
	int screen_index;
#if ENABLE_KMS
	kms_surface srf;
#elif ENABLE_DMX
#endif
	EGLSurface surface;
	//EGLContext context;
} EGL_WINDOW_T;

static EGL_SCREEN_T * screens = NULL;
static int num_screens = 0;
static EGL_WINDOW_T * windows = NULL;
static int num_windows = 0;
static EGLContext context = EGL_NO_CONTEXT;

void exit_egl();


int init_egl(int display_id) {

	int screen_index = num_screens;
	num_screens++;

	if (!screens)
		screens = (EGL_SCREEN_T*)malloc(sizeof(EGL_SCREEN_T));
	else
		screens = (EGL_SCREEN_T*)realloc(screens, sizeof(EGL_SCREEN_T) * num_screens);

	EGL_SCREEN_T * screen = &screens[screen_index];

#if ENABLE_KMS
	screen->output = create_output_kms(display_id);
	if (screen->output == NULL)
		return -1;

	screen->displayType = get_display_kms();
#else
	screen->displayType = EGL_DEFAULT_DISPLAY;
#endif

	screen->display = eglGetDisplay(screen->displayType);
	if (screen->display == EGL_NO_DISPLAY)
		return -1;

	EGLBoolean result;

	result = eglInitialize(screen->display, NULL, NULL);
	if (result == EGL_FALSE)
		return -2;

	static const EGLint attribute_list[] =
	{
	   EGL_RED_SIZE, 8,
	   EGL_GREEN_SIZE, 8,
	   EGL_BLUE_SIZE, 8,
	   EGL_ALPHA_SIZE, 8,
	   EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	   EGL_NONE
	};
	result = eglChooseConfig(screen->display, attribute_list, &screen->config, 1, &screen->num_config);
	if (result == EGL_FALSE)
		return -3;

	eglSwapInterval(screen->display, 1);

	return screen_index;
}

void res_egl(int screen_index, int * width, int * height) {
	EGL_SCREEN_T * screen = &screens[screen_index];
#if ENABLE_KMS
	get_resolution_kms(screen->output, width, height);
#endif
}

int window_egl(int screen_index, int width, int height) {

	EGL_SCREEN_T * screen = &screens[screen_index];

	int window_index = num_windows;
	num_windows++;

	if (!windows)
		windows = (EGL_WINDOW_T*)malloc(sizeof(EGL_WINDOW_T));
	else
		windows = (EGL_WINDOW_T*)realloc(windows, sizeof(EGL_WINDOW_T) * num_windows);

	EGL_WINDOW_T * window = &windows[window_index];

	window->screen_index = screen_index;

	static const EGLint context_attributes[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	if (context == EGL_NO_CONTEXT) {
		context = eglCreateContext(screen->display, screen->config, EGL_NO_CONTEXT, context_attributes);
		if (context == EGL_NO_CONTEXT)
			return -1;
	}

	NativeWindowType native_window;

#if ENABLE_KMS
	window->srf = create_surface_kms(width, height);
	native_window = (NativeWindowType)get_native_surface_kms(window->srf);
#else
	// TODO
#endif

	window->surface = eglCreateWindowSurface(screen->display, screen->config, native_window, NULL);
	if (window->surface == EGL_NO_SURFACE)
		return -3;

	EGLBoolean result;

	result = eglMakeCurrent(screen->display, window->surface, window->surface, context);
	if (result == EGL_FALSE)
		return -4;

	return window_index;
}

int loop_egl() {
	return 1;
}

void make_current_egl(int window_index) {
	EGL_WINDOW_T * window = &windows[window_index];
	EGL_SCREEN_T * screen = &screens[window->screen_index];
	if (screen->display == EGL_NO_DISPLAY || window->surface == EGL_NO_SURFACE)
		return;
	eglMakeCurrent(screen->display, window->surface, window->surface, context);
}

void size_egl(int window_index, int * width, int * height) {
	EGL_WINDOW_T * window = &windows[window_index];
    EGL_SCREEN_T * screen = &screens[window->screen_index];
	if (screen->display == EGL_NO_DISPLAY || window->surface == EGL_NO_SURFACE)
		return;
	eglQuerySurface(screen->display, window->surface, EGL_WIDTH, width);
    eglQuerySurface(screen->display, window->surface, EGL_HEIGHT, height);
}

void redraw_egl(int window_index) {
	EGL_WINDOW_T * window = &windows[window_index];
	EGL_SCREEN_T * screen = &screens[window->screen_index];
	if (screen->display == EGL_NO_DISPLAY || window->surface == EGL_NO_SURFACE)
		return;
	eglSwapBuffers(screens[window->screen_index].display, window->surface);
#if ENABLE_KMS
	int width, height;
	size_egl(window_index, &width, &height);
	draw_surface_kms(screen->output, window->srf, width, height);
#endif
}

void exit_egl() {
    
    for (int i=0; i<num_windows; i++) {
        EGL_WINDOW_T * window = &windows[i];
        EGL_SCREEN_T * screen = &screens[window->screen_index];
		if (screen->display != EGL_NO_DISPLAY && window->surface != EGL_NO_SURFACE)
			eglDestroySurface(screen->display, window->surface);
    }
    free(windows);

    for (int i=0; i<num_screens; i++) {
        EGL_SCREEN_T * screen = &screens[i];
		if (screen->display != EGL_NO_DISPLAY)
			eglTerminate(screen->display);
    }
    free(screens);
}
