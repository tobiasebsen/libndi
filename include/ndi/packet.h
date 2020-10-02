#ifndef NDI_PACKET_H
#define NDI_PACKET_H

#define NDI_DATA_TYPE_VIDEO		0
#define NDI_DATA_TYPE_AUDIO		1
#define NDI_DATA_TYPE_METADATA	2

typedef struct {
	unsigned long long timecode;
	unsigned int fourcc;
	unsigned int width;
	unsigned int height;
	unsigned int framerate_num;
	unsigned int framerate_den;
	unsigned char * data;
	int size;
} ndi_packet_video_t;

typedef struct {
	unsigned long long timecode;
	unsigned int fourcc;
	unsigned int num_samples;
	unsigned int num_channels;
	unsigned int sample_rate;
	unsigned char * data;
	int size;
} ndi_packet_audio_t;

typedef struct {
	unsigned long long timecode;
	char * data;
	int size;
} ndi_packet_metadata_t;

#endif // NDI_PACKET_H


