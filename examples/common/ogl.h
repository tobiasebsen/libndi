#pragma once

#ifdef __arm__
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GLES2/gl2.h"
#else
#include <GL/glew.h>
#endif

int init_glfw(int display_index);
int init_egl(int display_index);

void exit_glfw();
void exit_egl();

void res_glfw(int screen_index, int * width, int * height);
void res_egl(int screen_index, int * width, int * height);

int window_glfw(int screen_index, int width, int height);
int window_egl(int screen_index, int width, int height);

int loop_glfw();
int loop_egl();

void make_current_glfw(int window_index);
void make_current_egl(int window_index);

void size_glfw(int window_index, int * width, int * height);
void size_egl(int window_index, int * width, int * height);

void redraw_glfw(int window_index);
void redraw_egl(int window_index);

#ifdef __arm__
#define init_ogl	init_egl
#define exit_ogl	exit_egl
#define res_ogl		res_egl
#define window_ogl	window_egl
#define loop_ogl	loop_egl
#define make_current_ogl make_current_egl
#define size_ogl	size_egl
#define redraw_ogl	redraw_egl
#else
#define init_ogl	init_glfw
#define exit_ogl	exit_glfw
#define res_ogl		res_glfw
#define window_ogl	window_glfw
#define loop_ogl	loop_glfw
#define make_current_ogl make_current_glfw
#define size_ogl	size_glfw
#define redraw_ogl	redraw_glfw
#endif
