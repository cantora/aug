
#include "aug_plugin.h"
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <ccan/tap/tap.h>
#include <ccan/array_size/array_size.h>

const char aug_plugin_name[] = "unload";

static const struct aug_api *g_api;
static struct aug_plugin *g_plugin;

static pthread_t g_tid1;

static void *thread1(void *user) {
	(void)(user);

	diag("++++thread1++++");
	pass("thread1 running");
	ok1((*g_api->unload)(g_plugin) == 0);
	diag("----thread1----\n#");
	return NULL;
}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	plan_tests(4);
	diag("++++plugin_init++++");
	g_plugin = plugin;	
	g_api = api;

	(*g_api->log)(g_plugin, "init\n");

	diag("create thread for unloading");
	if(pthread_create(&g_tid1, NULL, thread1, NULL) != 0) {
		diag("expected to be able to create a thread. abort...");
		return -1;
	}
	pass("created thread");

	diag("----plugin_init----\n#");

	return 0;
}

void aug_plugin_free() {

	diag("++++plugin_free++++");
	(*g_api->log)(g_plugin, "free\n");

	ok1(pthread_self() == g_tid1);
	diag("----plugin_free----\n#");
}