#include <ndi/recv.h>
#include <ndi/scramble.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <unistd.h>
#endif

#if defined _WIN32
#if defined _WIN64
const char * _platformName = "WIN64";
#else
const char * _platformName = "WIN32";
#endif
#elif defined __linux__
const char * _platformName = "LINUX";
#else
const char * _platformName = "UNKNOWN";
#endif

extern int errno;

typedef struct {
	int socket_fd;
	fd_set read_fds;
} internal_recv_context_t;


ndi_recv_context_t ndi_recv_create() {
	internal_recv_context_t * ctx = malloc(sizeof(internal_recv_context_t));
	memset(ctx, 0, sizeof(internal_recv_context_t));
	return ctx;
}

static void internal_write_u16(void * buffer, int offset, unsigned short v) {
	unsigned char * data = ((unsigned char*)buffer) + offset;
	data[0] = v & 0xFF;
	data[1] = v >> 8;
}

static void internal_write_u32(void * buffer, int offset, unsigned int v) {
	unsigned char * data = ((unsigned char*)buffer) + offset;
	data[0] = v & 0xFF;
	data[1] = (v >> 8) & 0xFF;
	data[2] = (v >> 16) & 0xFF;
	data[3] = (v >> 24) & 0xFF;
}

static void internal_write_u64(void * buffer, int offset, unsigned long long v) {
	unsigned char * data = ((unsigned char*)buffer) + offset;
	for (int i = 0; i < 8; i++) {
		data[i] = (v & 0xFF);
		v >>= 8;
	}
}

static void internal_send_meta(ndi_recv_context_t ctx, char * data) {
	
	internal_recv_context_t * internal = ctx;
	int data_len = strlen(data) + 1;
	int len = 20 + data_len;
	unsigned char * buffer = malloc(len);

	internal_write_u16(buffer, 0, 0x8001);
	internal_write_u16(buffer, 2, NDI_DATA_TYPE_METADATA);
	internal_write_u32(buffer, 4, 8); // info length
	internal_write_u32(buffer, 8, data_len);
	internal_write_u64(buffer, 12, 0);

	memcpy(buffer + 20, data, data_len);

	ndi_scramble_type1(buffer + 12, 8 + data_len, 8 + data_len);

	send(internal->socket_fd, buffer, len, 0);

cleanup:
	free(buffer);
}

void ndi_recv_send_metadata(ndi_recv_context_t ctx, ndi_packet_metadata_t * meta) {
	internal_send_meta(ctx, meta->data);
}

int ndi_recv_connect(ndi_recv_context_t ctx, const char * host, unsigned short port) {

	internal_recv_context_t * internal = ctx;

#ifdef _WIN32
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif

	char port_str[10];
#ifdef _WIN32
	_itoa_s(port, port_str, sizeof(port_str), 10);
#else
	sprintf(port_str, "%d", port);
#endif

	int ret;
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	if ((ret = getaddrinfo(host, port_str, &hints, &res)) != 0) {
		return -1;
	}

	for (struct addrinfo * p = res; p != NULL; p = p->ai_next) {
		internal->socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (internal->socket_fd < 0)
			continue;

		ret = connect(internal->socket_fd, res->ai_addr, res->ai_addrlen);
		if (ret < 0) {
			freeaddrinfo(res);
			return -1;
		}
	}

	freeaddrinfo(res);

	if (!internal->socket_fd)
		return -1;

	char meta[100];

	sprintf(meta, "<ndi_version text=\"3\" video=\"4\" audio=\"3\" sdk=\"3.5.1\" platform=\"%s\"/>", _platformName);
	internal_send_meta(ctx, meta);

	sprintf(meta, "<ndi_video quality=\"high\"/>");
	internal_send_meta(ctx, meta);

	sprintf(meta, "<ndi_enabled_streams video=\"true\" audio=\"true\" text=\"true\"/>");
	internal_send_meta(ctx, meta);

	// <ndi_identify name=\"\"/>
	// <ndi_capabilities ntk_ptz="true" ntk_pan_tilt="true" ntk_zoom="true" ntk_iris="false" ntk_white_balance="false" ntk_exposure="false" ntk_record="false" web_control="" ndi_type="NDI"/>
	// <ndi_failover name=\"\" ip=\"\"/>
	// <ndi_product long_name=\"\" short_name=\"\" manufacturer=\"\" version=\"1.000.000\" session=\"default\" model_name=\"\" serial=\"\"/>

	return 0;
}

void ndi_recv_close(ndi_recv_context_t ctx) {

	internal_recv_context_t * internal = ctx;

	if (internal->socket_fd) {
#ifdef _WIN32
		closesocket(internal->socket_fd);
#else
		close(internal->socket_fd);
#endif
		internal->socket_fd = 0;
	}
}

int ndi_recv_is_connected(ndi_recv_context_t ctx) {
	internal_recv_context_t * internal = ctx;
	return internal->socket_fd > 0;
}

static unsigned char internal_read_u8(void * buffer, int offset) {
	unsigned char * data = ((unsigned char*)buffer) + offset;
	return data[0];
}

static unsigned short internal_read_u16(void * buffer, int offset) {
	unsigned char * data = ((unsigned char*)buffer) + offset;
	return data[0] | (data[1] << 8);
}

static unsigned int internal_read_u32(void * buffer, int offset) {
	unsigned char * data = ((unsigned char*)buffer) + offset;
	return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

static unsigned long long internal_read_u64(void * buffer, int offset) {
	unsigned char * data = ((unsigned char*)buffer) + offset;
	return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24) | ((unsigned long long)data[4] << 32) | ((unsigned long long)data[5] << 40) | ((unsigned long long)data[6] << 48) | ((unsigned long long)data[7] << 56);
}

static int internal_recv(int socket, char * buf, int len) {
	int rb = 0;
	while (rb < len) {
		int n = recv(socket, buf + rb, len - rb, 0);
		if (n < 0)
			return -1;
		rb += n;
	}
	return rb;
}

static void internal_unscramble(int type2, unsigned char *buf, int len, unsigned int seed) {
	if (!type2)
		ndi_unscramble_type1(buf, len, seed);
	else
		ndi_unscramble_type2(buf, len, seed);
}

int ndi_recv_wait(ndi_recv_context_t ctx, int timeout_ms) {

	internal_recv_context_t * internal = ctx;

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timeout_ms * 1000;

	FD_ZERO(&internal->read_fds);
	FD_SET(internal->socket_fd, &internal->read_fds);

	int status = select(internal->socket_fd + 1, &internal->read_fds, NULL, NULL, &tv);
	if (status <= 0) {
		if (errno == EBADF)
			ndi_recv_close(ctx);
		return -1;
	}

	return 0;
}

int ndi_recv_capture(ndi_recv_context_t ctx, ndi_packet_video_t * video, ndi_packet_audio_t * audio, ndi_packet_metadata_t * meta, int timeout_ms) {

	internal_recv_context_t * internal = ctx;
	int ret;

	ret = ndi_recv_wait(ctx, timeout_ms);
	if (ret < 0)
		return -1;

	int status = 0;
	int available = 0;

#ifdef _WIN32
	unsigned long n = 0;
	status = ioctlsocket(internal->socket_fd, FIONREAD, &n);
	available = n;
#else
	status = ioctl(internal->socket_fd, FIONREAD, &available);
#endif

	char header[12];

	if (status != 0 || available < 12) {
		int len = recv(internal->socket_fd, header, available, 0);
		if (strcmp(header, "_close/>") == 0) {
			ndi_recv_close(ctx);
		}
		return -1;
	}

	int len = recv(internal->socket_fd, header, 12, 0);
	if (len != 12)
		return -1;

	unsigned short version = internal_read_u8(header, 0);
	unsigned char id = internal_read_u8(header, 1);
	unsigned short packet_type = internal_read_u16(header, 2);
	unsigned int info_len = internal_read_u32(header, 4);
	unsigned int data_len = internal_read_u32(header, 8);
	unsigned int seed = info_len + data_len;

	if (id != 0x80)
		return -1;

	if (packet_type == NDI_DATA_TYPE_VIDEO && video) {

		unsigned char * info = malloc(info_len);
		internal_recv(internal->socket_fd, info, info_len);
		internal_unscramble(version > 3, info, info_len, seed);

		video->fourcc = internal_read_u32(info, 0);
		video->width = internal_read_u32(info, 4);
		video->height = internal_read_u32(info, 8);
		video->framerate_num = internal_read_u32(info, 12);
		video->framerate_den = internal_read_u32(info, 16);
		video->size = data_len;
		video->data = malloc(data_len);
		internal_recv(internal->socket_fd, video->data, data_len);

		free(info);
	}
	if (packet_type == NDI_DATA_TYPE_AUDIO && audio) {

		unsigned char * info = malloc(info_len);
		internal_recv(internal->socket_fd, info, info_len);
		internal_unscramble(version > 2, info, info_len, seed);

		audio->fourcc = internal_read_u32(info, 0);
		audio->num_samples = internal_read_u32(info, 4);
		audio->num_channels = internal_read_u32(info, 8);
		audio->sample_rate = internal_read_u32(info, 12);
		audio->size = data_len;
		audio->data = malloc(data_len);
		internal_recv(internal->socket_fd, audio->data, data_len);

		free(info);
	}
	if (packet_type == NDI_DATA_TYPE_METADATA && meta != NULL) {

		int chunk_len = info_len + data_len;
		unsigned char * info = malloc(chunk_len);
		internal_recv(internal->socket_fd, info, chunk_len);
		internal_unscramble(version > 2, info, chunk_len, seed);

		meta->timecode = internal_read_u64(info, 0);
		meta->data = malloc(data_len);
		meta->size = data_len;
		memcpy(meta->data, info + info_len, data_len);

		free(info);
	}

	return packet_type;
}

void ndi_recv_free_video(ndi_packet_video_t * video) {
	free(video->data);
	video->data = NULL;
}

void ndi_recv_free_audio(ndi_packet_audio_t * audio) {
	free(audio->data);
}

void ndi_recv_free_metadata(ndi_packet_metadata_t * meta) {
	free(meta->data);
}

void ndi_recv_free(ndi_recv_context_t ctx) {

	internal_recv_context_t * internal = ctx;

	ndi_recv_close(ctx);

	free(ctx);
}
