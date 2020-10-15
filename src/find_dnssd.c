#include <ndi/find.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h> //<-- REMOVE
#include <time.h>
#include <string.h>
#ifdef __APPLE__
#include <arpa/inet.h>
#endif

#include <dns_sd.h>

extern int errno;

typedef struct {
	DNSServiceRef sdRef;
	ndi_source_t * sources;
    int num_sources;
} internal_dnssd_t;

static int internal_sd_wait(DNSServiceRef sdRef, int timeout_ms) {

	int fd = DNSServiceRefSockFD(sdRef);
	if (fd == -1)
		return -1;

	fd_set read_fds;
	struct timeval tv;
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);

	int status = select(fd + 1, &read_fds, NULL, NULL, &tv);
	if (status <= 0)
		return -2;

	DNSServiceErrorType err = DNSServiceProcessResult(sdRef);
	if (err != kDNSServiceErr_NoError)
		return -3;

	return 0;
}

static void DNSSD_API internal_addr_reply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *hostname, const struct sockaddr *address, uint32_t ttl, void *context) {
	ndi_source_t * source = context;
	struct sockaddr_in *addr_in = (struct sockaddr_in *)address;

	//inet_ntop(AF_INET, &addr_in->sin_addr, service->host, sizeof(service->host));

	char *s = inet_ntoa(addr_in->sin_addr);
	strcpy(source->ip, s);

    printf("resolve ip: %s\n", s);
}

static void DNSSD_API internal_resolve_reply(DNSServiceRef s, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *fullname, const char *hosttarget, uint16_t port, uint16_t txtLen, const unsigned char  *txtRecord, void *context) {
    ndi_source_t * source = context;

	strncpy(source->host, hosttarget, sizeof(source->host));
	source->port = ntohs(port);

	DNSServiceRef sdRef;
	DNSServiceErrorType err = DNSServiceGetAddrInfo(&sdRef, 0, interfaceIndex, kDNSServiceProtocol_IPv4, hosttarget, internal_addr_reply, source);
	if (err == kDNSServiceErr_NoError) {
		internal_sd_wait(sdRef, 1000);
		DNSServiceRefDeallocate(sdRef);
	}
}

static void DNSSD_API internal_browse_reply(DNSServiceRef s, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regtype, const char *replyDomain, void *context) {
	internal_dnssd_t * internal = context;

    int source_index = internal->num_sources;
    internal->num_sources++;
    
    if (internal->sources == NULL)
        internal->sources = malloc(sizeof(ndi_source_t));
    else
        internal->sources = realloc(internal->sources, sizeof(ndi_source_t) * internal->num_sources);
    
    ndi_source_t * source = &internal->sources[source_index];
    strncpy(source->name, serviceName, sizeof(source->name));

    DNSServiceRef sdRef;
	DNSServiceErrorType err = DNSServiceResolve(&sdRef, 0, interfaceIndex, serviceName, regtype, replyDomain, internal_resolve_reply, source);
	if (err == kDNSServiceErr_NoError) {
		internal_sd_wait(sdRef, 1000);
		DNSServiceRefDeallocate(sdRef);
	}
}

ndi_find_context_t ndi_find_create() {

	internal_dnssd_t * internal = malloc(sizeof(internal_dnssd_t));
	if (!internal) return NULL;

	internal->sources = NULL;
    internal->num_sources = 0;

	DNSServiceBrowse(&internal->sdRef, 0, 0, "_ndi._tcp", 0, internal_browse_reply, internal);

	return internal;
}

ndi_source_t * ndi_find_sources(ndi_find_context_t ctx, int timeout_ms, int * num_sources) {

	internal_dnssd_t * internal = ctx;

	int ret = internal_sd_wait(internal->sdRef, timeout_ms);
    if (ret < 0) {
        printf("Wait failed: %d. Errno: %d\n", ret, errno);
    }

	if (num_sources != NULL)
		*num_sources = internal->num_sources;

	return internal->sources;
}

void ndi_find_free(ndi_find_context_t ctx) {

	if (!ctx) return;

	internal_dnssd_t * internal = ctx;
	DNSServiceRefDeallocate(internal->sdRef);

	free(internal->sources);

	free(ctx);
}
