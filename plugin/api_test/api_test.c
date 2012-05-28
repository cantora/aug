#include "aug_plugin.h"
#include <string.h>

const char aug_plugin_name[] = "api_test";

static const struct aug_api *g_api;
static struct aug_plugin *g_plugin;

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	const char *testkey;

	g_plugin = plugin;	
	g_api = api;

	(*g_api->log)(g_plugin, "init\n");

	if( (*g_api->conf_val)(g_plugin, aug_plugin_name, "testkey", &testkey) == 0) {
		(*g_api->log)(g_plugin, "testkey = %s\n", testkey);
		if(strcmp(testkey, "testval") != 0) {
			(*g_api->log)(g_plugin, "expected testkey = testval\n");
			return -1;
		}			
	}
	else {
		(*g_api->log)(g_plugin, "testkey not found. abort...\n");
		return -1;
	}

	
	return 0;
}

void aug_plugin_free() {
	(*g_api->log)(g_plugin, "free\n");
}