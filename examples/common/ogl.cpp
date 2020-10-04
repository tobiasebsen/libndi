#include "../common/ogl.h"

int init_ogl() {
#ifdef __arm__
	return init_egl();
#else
	return init_glfw();
#endif
}

void exit_ogl() {
#ifdef __arm__
	exit_egl();
#else
	exit_glfw();
#endif
}

int window_ogl(int screen_index, int width, int height) {
#ifdef __arm__
	return window_egl(screen_index, width, height);
#else
	return window_glfw(screen_index, width, height);
#endif
}

int loop_ogl() {
#ifdef __arm__
	return 1;
#else
	return loop_glfw();
#endif
}

void redraw_ogl(int window_index) {
#ifdef __arm__
	return redraw_egl(window_index);
#else
	return redraw_glfw(window_index);
#endif
}

void orth_ogl(float * matrix, float left, float right, float top, float bottom, float near, float far) {
	matrix[0] = 2.0f / (right - left);
	matrix[5] = 2.0f / (top - bottom);
	matrix[10] = -2.0f / (far - near);
	matrix[12] = -(right + left) / (right - left);
	matrix[13] = -(top + bottom) / (top - bottom);
	matrix[14] = -(far + near) / (far - near);
	matrix[15] = 1;
}

