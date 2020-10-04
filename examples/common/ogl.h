#pragma once

#ifdef __arm__
#include "GLES2/gl2.h"
#else
#include <GL/glew.h>
#endif

int init_glfw();
int init_egl();
int init_ogl();

void exit_glfw();
void exit_egl();
void exit_ogl();

int window_glfw(int screen_index, int width, int height);
int window_egl(int screen_index, int width, int height);
int window_ogl(int screen_index, int width, int height);

int loop_glfw();
int loop_egl();
int loop_ogl();

void redraw_glfw(int window_index);
void redraw_egl(int window_index);
void redraw_ogl(int window_index);

void orth_ogl(float * matrix, float left, float right, float top, float bottom, float near, float far);