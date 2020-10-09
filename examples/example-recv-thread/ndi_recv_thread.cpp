#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include <ndi.h>

void recv_thread(ndi_recv_context_t recv_ctx) {

	while (ndi_recv_is_connected(recv_ctx)) {

		ndi_packet_video_t video;
		ndi_packet_audio_t audio;
		ndi_packet_metadata_t meta;

		int data_type = ndi_recv_capture(recv_ctx, &video, &audio, &meta, 5000);
		switch (data_type) {

		case NDI_DATA_TYPE_VIDEO:
			printf("Video data received (%dx%d %.4s).\n", video.width, video.height, &video.fourcc);
			ndi_recv_free_video(&video);
			break;

		case NDI_DATA_TYPE_AUDIO:
			printf("Audio data received (%d samples).\n", audio.num_samples);
			ndi_recv_free_audio(&audio);
			break;

		case NDI_DATA_TYPE_METADATA:
			printf("Meta data received: %s\n", meta.data);
			ndi_recv_free_metadata(&meta);
			break;
		}
	}
}

int main(int argc, char* argv[]) {

	ndi_find_context_t find_ctx = ndi_find_create();

	ndi_source_t  * sources = NULL;
	int nb_sources = 0;
	while (!nb_sources) {
		printf("Looking for sources ...\n");
		sources = ndi_find_sources(find_ctx, 5000, &nb_sources);
	}

	printf("Found source: %s\n", sources[0].name);

	ndi_recv_context_t recv_ctx = ndi_recv_create();
	int ret = ndi_recv_connect(recv_ctx, sources[0].ip, sources[0].port);
	if (ret < 0)
		return -1;

	std::thread t(recv_thread, recv_ctx);

	ndi_find_free(find_ctx);

	getc(stdin);

	ndi_recv_close(recv_ctx);
	t.join();
	ndi_recv_free(recv_ctx);

	return 0;
}