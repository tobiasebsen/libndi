#ifndef NDI_RECV_H
#define NDI_RECV_H

#include <ndi/packet.h>

typedef void * ndi_recv_context_t;

ndi_recv_context_t ndi_recv_create();
void ndi_recv_free(ndi_recv_context_t ctx);

int ndi_recv_connect(ndi_recv_context_t ctx, const char * host, unsigned short port);
void ndi_recv_close(ndi_recv_context_t ctx);
int ndi_recv_is_connected(ndi_recv_context_t ctx);

int ndi_recv_capture(ndi_recv_context_t ctx, ndi_packet_video_t * video, ndi_packet_audio_t * audio, ndi_packet_metadata_t * meta, int timeout_ms);
void ndi_recv_free_video(ndi_packet_video_t * video);
void ndi_recv_free_audio(ndi_packet_audio_t * audio);
void ndi_recv_free_metadata(ndi_packet_metadata_t * meta);

int ndi_recv_send_metadata(ndi_recv_context_t ctx, ndi_packet_metadata_t * meta);

#endif // NDI_RECV_H
