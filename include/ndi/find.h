#ifndef NDI_FIND_H
#define NDI_FIND_H

typedef void * ndi_find_context_t;

typedef struct {
	char name[64];
	char host[256];
	char ip[16];
	unsigned short port;
} ndi_source_t;

ndi_find_context_t ndi_find_create();
ndi_source_t * ndi_find_sources(ndi_find_context_t ctx, int timeout_ms, int * num_sources);
void ndi_find_free(ndi_find_context_t ctx);

#endif // NDI_FIND_H
