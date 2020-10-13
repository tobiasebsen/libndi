#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ndi.h>

#ifdef __arm__
#include "bcm_host.h"
#endif

#include "../common/ogl.h"
#include "../common/yuv.h"


int main(int argc, char* argv[]) {

	int ret;

#ifdef __arm__
	bcm_host_init();
#endif

	if ((ret = init_ogl(0)) < 0) {
		printf("Failed to initialize OpenGL. Error: %d\n", ret);
		return -1;
	}

	int width, height;
	res_ogl(0, &width, &height);
	printf("Screen #0: %dx%d\n", width, height);

	if ((ret = window_ogl(0, width, height)) < 0) {
		printf("Failed to create window. Error: %d\n", ret);
		return -1;
	}

	glEnable(GL_TEXTURE_2D);
	glClearColor(0, 0, 0, 1);

	GLuint texture[4];
	glGenTextures(4, texture);

#ifndef __arm__
	glewInit();
#endif

	if (yuv_init() < 0) {
		printf("Failed to load shader\n");
		return -1;
	}

	yuv_bind();

	float matrix[16];
	GLfloat vertices[] = { 0, 0, 0, 1920, 0, 0, 1920, 1080, 0, 0, 1080, 0 };
	GLfloat tex_coords[] = { 0, 0, 1, 0, 1, 1, 0, 1 };

	memset(matrix, 0, sizeof(matrix));
	orth_ogl(&matrix[0], 0, 1920, 0, 1080, -1, 1);

	int loc_m = yuv_get_uniform("u_matrix");
	int loc_p = yuv_get_attrib("a_position");
	int loc_t = yuv_get_attrib("a_texcoord");

	//

	ndi_find_context_t find_ctx = ndi_find_create();

	ndi_source_t  * sources = NULL;
	int nb_sources = 0;
	while (!nb_sources) {
		printf("Looking for sources ...\n");
		sources = ndi_find_sources(find_ctx, 5000, &nb_sources);
	}

	printf("Found source: %s (%s:%d)\n", sources[0].name, sources[0].ip, sources[0].port);

	ndi_recv_context_t recv_ctx = ndi_recv_create();
	ret = ndi_recv_connect(recv_ctx, sources[0].ip, sources[0].port);
	if (ret < 0) {
		printf("Failed to connect to source\n");
		return -1;
	}

	ndi_find_free(find_ctx);

	ndi_codec_context_t codec_ctx = ndi_codec_create();

	while (loop_ogl()) {

		int width, height;
		size_ogl(0, &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

		ndi_packet_video_t video;
		ndi_packet_audio_t audio;
		ndi_packet_metadata_t meta;
		ndi_frame_t frame;
		ndi_video_format_t format;

		int data_type = ndi_recv_capture(recv_ctx, &video, &audio, &meta, 5000);
		switch (data_type) {

		case NDI_DATA_TYPE_VIDEO:
			printf("Video data received (%dx%d).\n", video.width, video.height);
			frame = ndi_codec_decode(codec_ctx, &video);
			if (frame) {
				ndi_frame_get_format(frame, &format);
				for (int i = 0; i < format.num_planes; i++) {
					int w = i ? format.chroma_width : format.width;
					int h = i ? format.chroma_height : format.height;
					glActiveTexture(GL_TEXTURE0 + i);
					glBindTexture(GL_TEXTURE_2D, texture[i]);
					//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video.width, video.height, GL_LUMINANCE, GL_UNSIGNED_BYTE, ndi_frame_data(frame, 0));
					glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ndi_frame_get_data(frame, i));
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				ndi_frame_free(frame);
			}
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

		yuv_textures(texture[0], texture[1], texture[2]);

		glUniformMatrix4fv(loc_m, 1, GL_FALSE, &matrix[0]);

		glVertexAttribPointer(loc_p, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), vertices);
		glEnableVertexAttribArray(loc_p);

		glVertexAttribPointer(loc_t, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), tex_coords);
		glEnableVertexAttribArray(loc_t);

		GLushort indices[] = { 0, 3, 1, 2 };
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices);

		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);

		redraw_ogl(0);
	}

	ndi_recv_free(recv_ctx);

	exit_ogl();

	return 0;
}