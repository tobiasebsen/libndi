#include <ndi/codec.h>

#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>

typedef struct {
	AVCodec * avcodec;
	AVCodecContext * avcodec_ctx;
} internal_codec_context_t;

ndi_codec_context_t ndi_codec_create() {
	internal_codec_context_t * internal = malloc(sizeof(internal_codec_context_t));
	internal->avcodec = NULL;
	internal->avcodec_ctx = NULL;
	return internal;
}

int ndi_codec_open(ndi_codec_context_t ctx, unsigned int fourcc) {
	
	internal_codec_context_t * internal = ctx;

	const struct AVCodecTag *table[] = { avformat_get_riff_video_tags(), 0 };
	enum AVCodecID codec_id = av_codec_get_id(table, fourcc);

	internal->avcodec = avcodec_find_decoder(codec_id);
	if (internal->avcodec == NULL)
		return -1;

	if (internal->avcodec_ctx != NULL)
		avcodec_free_context(&internal->avcodec_ctx);

	internal->avcodec_ctx = avcodec_alloc_context3(internal->avcodec);
	internal->avcodec_ctx->codec_tag = fourcc;

	int error = avcodec_open2(internal->avcodec_ctx, internal->avcodec, NULL);
	if (error < 0)
		return -1;

	return 0;
}

ndi_frame_t ndi_codec_decode(ndi_codec_context_t ctx, ndi_packet_video_t * video) {

	internal_codec_context_t * internal = ctx;

	int error;

	if (internal->avcodec_ctx == NULL || internal->avcodec_ctx->codec_tag != video->fourcc)
		ndi_codec_open(ctx, video->fourcc);

	AVPacket packet;
	av_init_packet(&packet);
	packet.data = video->data;
	packet.size = video->size;

	internal->avcodec_ctx->width = video->width;
	internal->avcodec_ctx->height = video->height;

	error = avcodec_send_packet(internal->avcodec_ctx, &packet);
	if (error < 0) {
		char * str = av_err2str(error);
		printf("Codec error: %s\n", str);
		return NULL;
	}

	AVFrame * frame = av_frame_alloc();
	error = avcodec_receive_frame(internal->avcodec_ctx, frame);
	if (error != 0) {
		av_frame_free(&frame);
		return NULL;
	}

	frame->opaque = internal->avcodec_ctx;

	return frame;
}

void ndi_codec_free(ndi_codec_context_t ctx) {
	internal_codec_context_t * internal = ctx;
	avcodec_free_context(&internal->avcodec_ctx);
	free(internal);
}

void * ndi_frame_get_data(ndi_frame_t f, int plane) {
	AVFrame * frame = f;
	return frame->data[plane];
}

int ndi_frame_get_linesize(ndi_frame_t f, int plane) {
	AVFrame * frame = f;
	return frame->linesize[plane];
}

void ndi_frame_get_format(ndi_frame_t f, ndi_video_format_t * format) {
	AVFrame * frame = f;
	AVCodecContext * codec_ctx = frame->opaque;
	format->width = frame->width;
	format->height = frame->height;

	format->num_planes = av_pix_fmt_count_planes(frame->format);

	const AVPixFmtDescriptor * fmt_dsc = av_pix_fmt_desc_get(frame->format);
	format->chroma_width = AV_CEIL_RSHIFT(frame->width, fmt_dsc->log2_chroma_w);
	format->chroma_height = AV_CEIL_RSHIFT(frame->height, fmt_dsc->log2_chroma_h);
}

void ndi_frame_free(ndi_frame_t f) {
	AVFrame * frame = f;
	av_frame_free(&frame);
}
