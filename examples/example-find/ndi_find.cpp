#include <stdio.h>
#include <chrono>
#include <ndi.h>

int main(int argc, char* argv[]) {

	ndi_find_context_t find_ctx = ndi_find_create();

	ndi_source_t  * sources = NULL;
	int nb_sources = 0;
	printf("Looking for sources ...\n");
	for (const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now(); std::chrono::high_resolution_clock::now() - start < std::chrono::seconds(10);) {
		sources = ndi_find_sources(find_ctx, 5000, &nb_sources);
		printf("Found %d source(s)\n", nb_sources);
		for (int i = 0; i < nb_sources; i++) {
			printf(" name: %s\n", sources[i].name);
			printf(" host: %s\n", sources[i].host);
			printf(" ip:   %s\n", sources[i].ip);
			printf(" port: %d\n", sources[i].port);
		}
	}

	ndi_find_free(find_ctx);

	return 0;
}
