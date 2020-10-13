#ifdef __arm__
#include <EGL/egl.h>

typedef struct {
	int display_index;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	unsigned int width;
	unsigned int height;
	EGLDisplay display;
	EGLConfig config;
	EGLint num_config;
} EGL_SCREEN_T;

typedef struct {
	int screen_index;
	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	EGL_DISPMANX_WINDOW_T nativewindow;
	EGLSurface surface;
	EGLContext context;
	int width;
	int height;
} EGL_WINDOW_T;

static EGL_SCREEN_T * screens = NULL;
static int num_screens = 0;
static EGL_WINDOW_T * windows = NULL;
static int num_windows = 0;

void exit_egl();


int init_egl(int display_index) {

	int screen_index = num_screens;
	num_screens++;

	if (!screens)
		screens = (EGL_SCREEN_T*)malloc(sizeof(EGL_SCREEN_T));
	else
		screens = (EGL_SCREEN_T*)realloc(screens, sizeof(EGL_SCREEN_T) * num_screens);

	EGL_SCREEN_T * screen = &screens[screen_index];

	screen->display_index = display_index;
	screen->dispman_display = vc_dispmanx_display_open(display_index);

	DISPMANX_MODEINFO_T info;
	vc_dispmanx_display_get_info(screen->dispman_display, &info);
	screen->width = info.width;
	screen->height = info.height;

	screen->display = eglGetDisplay((EGLNativeDisplayType)screen_index);
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

	screen->dispman_update = vc_dispmanx_update_start(display_index);

	eglSwapInterval(screen->display, 1);

	return screen_index;
}

void res_egl(int screen_index, int * width, int * height) {
	EGL_SCREEN_T * screen = &screens[screen_index];
	*width = screen->width;
	*height = screen->height;
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
	window->width = width;
	window->height = height;

	static const EGLint context_attributes[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	window->context = eglCreateContext(screen->display, screen->config, EGL_NO_CONTEXT, context_attributes);
	if (window->context == EGL_NO_CONTEXT)
		return -1;

	VC_RECT_T dst_rect = { 0, 0, (int)width, (int)height };
	VC_RECT_T src_rect = { 0, 0, (int)(width) << 16, (int)(height << 16) };
	window->dispman_element = vc_dispmanx_element_add(screen->dispman_update, screen->dispman_display, 0/*layer*/, &dst_rect, 0/*src*/, &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, DISPMANX_NO_ROTATE);
	if (window->dispman_element <= 0)
		return -2;

	vc_dispmanx_update_submit_sync(screen->dispman_update);

	window->nativewindow.element = window->dispman_element;
	window->nativewindow.width = width;
	window->nativewindow.height = height;
	window->surface = eglCreateWindowSurface(screen->display, screen->config, &window->nativewindow, NULL);
	if (window->surface == EGL_NO_SURFACE)
		return -3;

	EGLBoolean result;

	result = eglMakeCurrent(screen->display, window->surface, window->surface, window->context);
	if (result == EGL_FALSE)
		return -4;

	return window_index;
}

int loop_egl() {
	return 1;
}

void size_egl(int window_index, int * width, int * height) {
	EGL_WINDOW_T * window = &windows[window_index];
	*width = window->width;
	*height = window->height;
}

void redraw_egl(int window_index) {
	EGL_WINDOW_T * window = &windows[window_index];
	eglSwapBuffers(screens[window->screen_index].display, window->surface);
}

void exit_egl() {
}

#endif // __arm__