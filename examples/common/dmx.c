#if ENABLE_DMX

#include "bcm_host.h"

typedef struct {
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
} DMX_SCREEN_T;

typedef struct {
	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	EGL_DISPMANX_WINDOW_T nativewindow;
} DMX_WINDOW_T;

int init_dmx(int display_id) {

	screen->display_id = display_id;
	screen->dispman_display = vc_dispmanx_display_open(display_id);

	DISPMANX_MODEINFO_T info;
	vc_dispmanx_display_get_info(screen->dispman_display, &info);
	screen->width = info.width;
	screen->height = info.height;

	screen->dispman_update = vc_dispmanx_update_start(0);
}

int window_dmx() {

	VC_RECT_T dst_rect = { 0, 0, (int)width, (int)height };
	VC_RECT_T src_rect = { 0, 0, (int)(width) << 16, (int)(height << 16) };
	window->dispman_element = vc_dispmanx_element_add(screen->dispman_update, screen->dispman_display, 0/*layer*/, &dst_rect, 0/*src*/, &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, DISPMANX_NO_ROTATE);
	if (window->dispman_element <= 0)
		return -2;

	vc_dispmanx_update_submit_sync(screen->dispman_update);

	window->nativewindow.element = window->dispman_element;
	window->nativewindow.width = width;
	window->nativewindow.height = height;

}

#endif