#include "aug_plugin.h"
#include <stdlib.h>
#include <ccan/tap/tap.h>

const char aug_plugin_name[] = "fail_init";

static const struct aug_api *g_api;
static struct aug_plugin *g_plugin;

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {

	g_plugin = plugin;	
	g_api = api;
		
	return -1; /* fail to init */
}

void aug_plugin_free() {
	(*g_api->log)(g_plugin, "shouldnt have gotten here!\n");
	fail("init failed so execution shouldnt have gotten to this point");
}