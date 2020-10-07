#include <ndi/find.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

typedef struct internal_service_t {
	char name[64];
	char host[256];
	char ip[16];
	unsigned short port;
	struct internal_service_t * next;
} internal_service_t;

typedef struct {
	AvahiSimplePoll *simple_poll;
	AvahiClient *client;
	AvahiServiceBrowser *sb;
	AvahiServiceResolver * resolver;
	struct {
		internal_service_t * head;
		internal_service_t * tail;
		int count;
	} services;
	ndi_source_t * sources;
} internal_avahi_t;

static void resolve_callback(
	AvahiServiceResolver *r,
	AVAHI_GCC_UNUSED AvahiIfIndex interface,
	AVAHI_GCC_UNUSED AvahiProtocol protocol,
	AvahiResolverEvent event,
	const char *name,
	const char *type,
	const char *domain,
	const char *host_name,
	const AvahiAddress *address,
	uint16_t port,
	AvahiStringList *txt,
	AvahiLookupResultFlags flags,
	AVAHI_GCC_UNUSED void* userdata) {

	internal_service_t * service = userdata;

	if (event == AVAHI_RESOLVER_FOUND) {
		avahi_address_snprint(service->ip, sizeof(service->ip), address);
		strcpy(service->host, host_name);
		service->port = port;
	}
}

static void browse_callback(
	AvahiServiceBrowser *b,
	AvahiIfIndex interface,
	AvahiProtocol protocol,
	AvahiBrowserEvent event,
	const char *name,
	const char *type,
	const char *domain,
	AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
	void* userdata) {

	internal_avahi_t * internal = userdata;

	if (event == AVAHI_BROWSER_NEW) {
		internal_service_t * service = malloc(sizeof(internal_service_t));
		memset(service, 0, sizeof(internal_service_t));
		strcpy(service->name, name);
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

		internal->resolver = avahi_service_resolver_new(internal->client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, 0, resolve_callback, service);
		//if (!resolver)
		//	printf("Failed to resolve service '%s': %s\n", name, avahi_strerror(avahi_client_errno(internal->client)));
	}
}

static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
}

ndi_find_context_t ndi_find_create() {

	int error;

	internal_avahi_t * internal = malloc(sizeof(internal_avahi_t));
	if (!internal) return NULL;

	memset(internal, 0, sizeof(internal_avahi_t));

	internal->simple_poll = avahi_simple_poll_new();
	if (!internal->simple_poll)
		return NULL;

	internal->client = avahi_client_new(avahi_simple_poll_get(internal->simple_poll), 0, client_callback, NULL, &error);
	if (!internal->client)
		return NULL;

	internal->sb = avahi_service_browser_new(internal->client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_ndi._tcp", NULL, 0, browse_callback, internal);
	if (!internal->sb)
		goto fail;

	return internal;

fail:
	if (internal->sb)
		avahi_service_browser_free(internal->sb);

	if (internal->client)
		avahi_client_free(internal->client);

	if (internal->simple_poll)
		avahi_simple_poll_free(internal->simple_poll);

	return NULL;
}

static void internal_services_free(ndi_find_context_t ctx) {
	internal_avahi_t * internal = ctx;
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

	internal_avahi_t * internal = ctx;

	int r = avahi_simple_poll_iterate(internal->simple_poll, timeout_ms);

	if (internal->sources == NULL)
		internal->sources = malloc(internal->services.count * sizeof(ndi_source_t));
	else
		internal->sources = realloc(internal->sources, internal->services.count * sizeof(ndi_source_t));

	internal_service_t * service = internal->services.head;
	int i = 0;
	for (; service != NULL; i++) {
		strcpy(internal->sources[i].name, service->name);
		strcpy(internal->sources[i].host, service->host);
		strcpy(internal->sources[i].ip, service->ip);
		internal->sources[i].port = service->port;
		service = service->next;
	}

	if (num_sources != NULL)
		*num_sources = i;

	return internal->sources;
}

void ndi_find_free(ndi_find_context_t ctx) {

	internal_avahi_t * internal = ctx;
	if (!internal) return;

	free(internal->sources);

	internal_services_free(ctx);

	if (internal->resolver)
		avahi_service_resolver_free(internal->resolver);
	if (internal->sb)
		avahi_service_browser_free(internal->sb);
	if (internal->client)
		avahi_client_free(internal->client);
	if (internal->simple_poll)
		avahi_simple_poll_free(internal->simple_poll);
}
