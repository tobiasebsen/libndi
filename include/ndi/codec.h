#ifndef NDI_CODEC_H
#define NDI_CODEC_H

#include <ndi/packet.h>

typedef void * ndi_codec_context_t;
typedef void * ndi_frame_t;

typedef struct {
	int width;
	int height;
	int num_planes;
	int chroma_width;
	int chroma_height;
} ndi_video_format_t;

ndi_codec_context_t ndi_codec_create();
void ndi_codec_free(ndi_codec_context_t ctx);

int ndi_codec_open(ndi_codec_context_t ctx, unsigned int fourcc);
ndi_frame_t ndi_codec_decode(ndi_codec_context_t ctx, ndi_packet_video_t * video);
void ndi_frame_get_format(ndi_frame_t frame, ndi_video_format_t * format);
void * ndi_frame_get_data(ndi_frame_t frame, int plane);
int ndi_frame_get_linesize(ndi_frame_t frame, int plane);
void ndi_frame_free(ndi_frame_t frame);

#endif // NDI_CODEC_H
