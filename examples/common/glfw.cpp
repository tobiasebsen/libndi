#ifndef __arm__
#include <stdio.h>
#include <memory>
#include <GLFW/glfw3.h>

typedef struct {
	GLFWmonitor * monitor;
	int width;
	int height;
} GLFW_SCREEN_T;

typedef struct {
	GLFWwindow * window;
} GLFW_WINDOW_T;


static GLFW_SCREEN_T * screens = NULL;
static int num_screens = 0;
static GLFW_WINDOW_T * windows = NULL;
static int num_windows = 0;


void exit_glfw();

int init_glfw(int display_index) {

	GLFWmonitor ** monitors;

	if (!glfwInit())
		goto fail;

	monitors = glfwGetMonitors(&num_screens);
	if (num_screens == 0 || monitors == NULL)
		goto fail;

	screens = (GLFW_SCREEN_T*)malloc(sizeof(GLFW_SCREEN_T) * num_screens);

	for (int i = 0; i < num_screens; i++) {
		screens[i].monitor = monitors[i];
		glfwGetMonitorWorkarea(monitors[i], NULL, NULL, &screens[i].width, &screens[i].height);
	}

success:
	return num_screens;

fail:
	const char* description;
	glfwGetError(&description);
	printf("Error: %s\n", description);

	exit_glfw();

	return -1;
}

void res_glfw(int screen_index, int * width, int * height) {
    glfwGetMonitorWorkarea(screens[screen_index].monitor, NULL, NULL, width, height);
}

int window_glfw(int screen_index, int width, int height) {

	if (screen_index >= num_screens)
		return -1;

	int window_index = num_windows;
	num_windows++;

	if (windows == NULL)
		windows = (GLFW_WINDOW_T*)malloc(sizeof(GLFW_WINDOW_T));
	else
		windows = (GLFW_WINDOW_T*)realloc(windows, sizeof(GLFW_WINDOW_T) * num_windows);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	windows[window_index].window = glfwCreateWindow(width, height, "NDI", NULL, NULL);
	glfwMakeContextCurrent(windows[window_index].window);
	glfwSwapInterval(1);

	return window_index;
}

int loop_glfw() {

	//glfwWaitEvents();

	for (int i = 0; i < num_windows; i++) {
		if (glfwWindowShouldClose(windows[i].window))
			return 0;
	}

	glfwPollEvents();

	return 1;
}

void size_glfw(int window_index, int * width, int * height) {

    glfwGetFramebufferSize(windows[window_index].window, width, height);
}

void redraw_glfw(int window_index) {
	glfwSwapBuffers(windows[window_index].window);
}

void exit_glfw() {

	if (screens) {
		free(screens);
		screens = NULL;
	}

	glfwTerminate();
}

#endif // __arm__
