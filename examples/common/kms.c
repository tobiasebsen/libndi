#include "kms.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <gbm.h>

typedef struct {
	drmModeConnector * connector;
	drmModeEncoder * encoder;
	drmModeCrtc * crtc;
} KMS_OUTPUT_T;

typedef struct {
	struct gbm_surface * gbmSurface;
	struct gbm_bo *previousBo;
	uint32_t previousFb;
} KMS_SURFACE_T;

int device = 0;
drmModeRes * resources = NULL;
struct gbm_device * gbmDevice = 0;

int init_kms(int card_index) {

	char path[16];

	sprintf(path, "/dev/dri/card%d", card_index);

	device = open(path, O_RDWR | O_CLOEXEC);
	if (device < 0)
		return -1;

	resources = drmModeGetResources(device);
	if (!resources)
		return -2;

	gbmDevice = gbm_create_device(device);
	if (!gbmDevice)
		return -3;

	return 0;
}

void free_kms() {
	gbm_device_destroy(gbmDevice);
	drmModeFreeResources(resources);
	close(device);
}

void* get_display_kms() {
	return gbmDevice;
}

kms_output create_output_kms(int display_index) {

	if (!resources)
		if (init_kms(1) < 0)
			if (init_kms(0) < 0)
				return NULL;

	KMS_OUTPUT_T * output = malloc(sizeof(KMS_OUTPUT_T));

	if (display_index >= resources->count_connectors)
		return NULL;

	output->connector = drmModeGetConnector(device, resources->connectors[display_index]);
	if (!output->connector)
		return NULL;

	output->encoder = drmModeGetEncoder(device, output->connector->encoder_id);
	if (!output->encoder)
		return NULL;

	output->crtc = drmModeGetCrtc(device, output->encoder->crtc_id);
	if (!output->crtc)
		return NULL;

	return output;
}

void get_resolution_kms(kms_output o, int * width, int * height) {
	KMS_OUTPUT_T * output = (KMS_OUTPUT_T*)o;
	*width = output->connector->modes[0].hdisplay;
	*height = output->connector->modes[0].vdisplay;
}

kms_surface create_surface_kms(int hdisplay, int vdisplay) {
	KMS_SURFACE_T * surface = malloc(sizeof(KMS_SURFACE_T));
	surface->gbmSurface = gbm_surface_create(gbmDevice, hdisplay, vdisplay, GBM_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	return surface;
}

void * get_native_surface_kms(kms_surface s) {
	KMS_SURFACE_T * surface = (KMS_SURFACE_T*)s;
	return surface->gbmSurface;
}

void draw_surface_kms(kms_output o, kms_surface s, int hdisplay, int vdisplay) {
	KMS_OUTPUT_T * output = (KMS_OUTPUT_T*)o;
	KMS_SURFACE_T * surface = (KMS_SURFACE_T*)s;
	struct gbm_bo *bo = gbm_surface_lock_front_buffer(surface->gbmSurface);
	uint32_t handle = gbm_bo_get_handle(bo).u32;
	uint32_t pitch = gbm_bo_get_stride(bo);
	uint32_t fb;
	drmModeAddFB(device, hdisplay, vdisplay, 24, 32, pitch, handle, &fb);
	drmModeSetCrtc(device, output->crtc->crtc_id, fb, 0, 0, &output->connector->connector_id, 1, &output->connector->modes[0]);

	if (surface->previousBo) {
		drmModeRmFB(device, surface->previousFb);
		gbm_surface_release_buffer(surface->gbmSurface, surface->previousBo);
	}
	surface->previousBo = bo;
	surface->previousFb = fb;
}