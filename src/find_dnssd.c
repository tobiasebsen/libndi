#include <ndi/find.h>

#include <stdlib.h>
#include <time.h>

#include <dns_sd.h>

typedef struct {
	char name[64];
	char host[256];
	char ip[16];
	unsigned short port;
	struct internal_service_t * next;
} internal_service_t;

typedef struct {
	DNSServiceRef sdRef;
	struct {
		internal_service_t * head;
		internal_service_t * tail;
		int count;
	} services;
	ndi_source_t * sources;
} internal_dnssd_t;

static int internal_sd_wait(DNSServiceRef sdRef, int timeout_ms) {

	int fd = DNSServiceRefSockFD(sdRef);
	if (fd == -1)
		return -1;

	fd_set read_fds;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timeout_ms * 1000;

	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);

	int status = select(fd + 1, &read_fds, NULL, NULL, &tv);
	if (status <= 0)
		return -1;

	DNSServiceErrorType err = DNSServiceProcessResult(sdRef);
	if (err != kDNSServiceErr_NoError)
		return -1;

	return 0;
}

static void DNSSD_API internal_addr_reply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *hostname, const struct sockaddr *address, uint32_t ttl, void *context) {
	internal_service_t * service = context;
	struct sockaddr_in *addr_in = (struct sockaddr_in *)address;

#ifdef _WIN32
#elif
	inet_ntop(AF_INET, &addr_in->sin_addr, service->host, sizeof(service->host));
#endif

	char *s = inet_ntoa(addr_in->sin_addr);
	strcpy_s(service->ip, sizeof(service->ip), s);
}

static void DNSSD_API internal_resolve_reply(DNSServiceRef s, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *fullname, const char *hosttarget, uint16_t port, uint16_t txtLen, const unsigned char  *txtRecord, void *context) {
	internal_service_t * service = context;

	strcpy_s(service->host, sizeof(service->host), hosttarget);
	service->port = ntohs(port);

	DNSServiceRef sdRef;
	DNSServiceErrorType err = DNSServiceGetAddrInfo(&sdRef, 0, interfaceIndex, kDNSServiceProtocol_IPv4, hosttarget, internal_addr_reply, context);
	if (err == kDNSServiceErr_NoError) {
		internal_sd_wait(sdRef, 1000);
		DNSServiceRefDeallocate(sdRef);
	}
}

static void DNSSD_API internal_browse_reply(DNSServiceRef s, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regtype, const char *replyDomain, void *context) {
	internal_dnssd_t * internal = context;

	internal_service_t * service = malloc(sizeof(internal_service_t));
	strcpy_s(service->name, sizeof(service->name), serviceName);
	service->next = NULL;

	if (internal->services.head == NULL) {
		internal->services.head = service;
		internal->services.tail = service;
	}
	else {
		internal->services.tail->next = service;
		internal->services.tail = service;
	}
	internal->services.count++;

	DNSServiceRef sdRef;
	DNSServiceErrorType err = DNSServiceResolve(&sdRef, 0, interfaceIndex, serviceName, regtype, replyDomain, internal_resolve_reply, service);
	if (err == kDNSServiceErr_NoError) {
		internal_sd_wait(sdRef, 1000);
		DNSServiceRefDeallocate(sdRef);
	}
}

ndi_find_context_t ndi_find_create() {

	ndi_find_context_t * ctx = malloc(sizeof(internal_dnssd_t));
	if (!ctx) return NULL;

	internal_dnssd_t * internal = ctx;

	internal->services.head = NULL;
	internal->services.tail = NULL;
	internal->services.count = 0;

	internal->sources = NULL;

	DNSServiceBrowse(&internal->sdRef, 0, 0, "_ndi._tcp", 0, internal_browse_reply, ctx);

	return ctx;
}

static void internal_services_free(ndi_find_context_t ctx) {
	internal_dnssd_t * internal = ctx;
	if (internal->services.head == NULL) return;
	for (;;) {
		internal_service_t * next = internal->services.head->next;
		free(internal->services.head);
		internal->services.head = next;
		if (next == NULL)
			break;
	}
	internal->services.head = NULL;
	internal->services.tail = NULL;
	internal->services.count = 0;
}

ndi_source_t * ndi_find_sources(ndi_find_context_t ctx, int timeout_ms, int * num_sources) {

	internal_dnssd_t * internal = ctx;

	internal_sd_wait(internal->sdRef, timeout_ms);

	if (internal->sources == NULL)
		internal->sources = malloc(internal->services.count * sizeof(ndi_source_t));
	else
		internal->sources = realloc(internal->sources, internal->services.count * sizeof(ndi_source_t));

	internal_service_t * service = internal->services.head;
	int i = 0;
	for (; service != NULL; i++) {
		strcpy_s(internal->sources[i].name, sizeof(internal->sources[i].name), service->name);
		strcpy_s(internal->sources[i].host, sizeof(internal->sources[i].host), service->host);
		strcpy_s(internal->sources[i].ip, sizeof(internal->sources[i].ip), service->ip);
		internal->sources[i].port = service->port;
		service = service->next;
	}

	if (num_sources != NULL)
		*num_sources = i;

	return internal->sources;
}

void ndi_find_free(ndi_find_context_t ctx) {

	if (!ctx) return;

	internal_dnssd_t * internal = ctx;
	DNSServiceRefDeallocate(internal->sdRef);

	free(internal->sources);

	internal_services_free(ctx);

	free(ctx);
}