#include <stdio.h>
#include <chrono>
#include <ndi.h>

int main(int argc, char* argv[]) {

	ndi_recv_context_t recv_ctx = ndi_recv_create();
	int ret = ndi_recv_connect(recv_ctx, argv[1], atoi(argv[2]));
	if (ret < 0)
		return -1;

	ndi_codec_context_t codec_ctx = ndi_codec_create();

	for (const auto start = std::chrono::high_resolution_clock::now(); std::chrono::high_resolution_clock::now() - start < std::chrono::minutes(1); ) {

		ndi_packet_video_t video;
		ndi_packet_audio_t audio;
		ndi_packet_metadata_t meta;

		int data_type = ndi_recv_capture(recv_ctx, &video, &audio, &meta, 5000);
		switch (data_type) {

		case NDI_DATA_TYPE_VIDEO:
			printf("Video data received (%dx%d).\n", video.width, video.height);
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

	ndi_recv_free(recv_ctx);

	return 0;
}