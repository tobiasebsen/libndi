#include <stdio.h>

#include "../common/ogl.h"

int main(int argc, char* argv[]) {

	printf("Hello RPi4\n");

	int width, height;

	if (init_egl(0) < 0)
		return -1;

	res_egl(0, &width, &height);
	printf("Screen #0: %dx%d\n", width, height);



	if (init_egl(1) < 0)
		return -1;

	res_egl(1, &width, &height);
	printf("Screen #1: %dx%d\n", width, height);

	return 0;
}
