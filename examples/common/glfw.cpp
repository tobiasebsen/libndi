#include <GLFW/glfw3.h>

typedef struct {
	GLFWwindow* window;
} STATE_T;

STATE_T screen[2];


int init_glfw() {

	if (!glfwInit())
		goto fail;

success:
	return 0;
fail:
	const char* description;
	glfwGetError(&description);
	printf("Error: %s\n", description);

	glfwTerminate();
	return -1;
}

void redraw_glfw() {
}

void exit_glfw() {
	glfwTerminate();
}
