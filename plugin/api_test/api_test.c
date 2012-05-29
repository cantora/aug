#include "aug_plugin.h"
#include <string.h>

const char aug_plugin_name[] = "api_test";

static const struct aug_api *g_api;
static struct aug_plugin *g_plugin;

static void on_r(int chr, void *user) {
	(*g_api->log)(g_plugin, "callback on '%c'. user data = %s\n", chr, user);
}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	const char *testkey;
	int stack_size = -1;
	int pos = -1;

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

	(*g_api->stack_size)(g_plugin, &stack_size);
	if(stack_size != 3) {
		(*g_api->log)(g_plugin, "expected stack size = 3. abort...\n");
		return -1;
	}

	if( (*g_api->stack_pos)(g_plugin, g_plugin->name, &pos) != 0) {
		(*g_api->log)(g_plugin, "should have found self at a plugin stack position. abort...\n");
		return -1;
	}
	
	if( pos != 1) {
		(*g_api->log)(g_plugin, "expected to be at stack position 1. abort...\n");
		return -1;
	}
	
	/* register a callback for when the user types command key + 'r' */
	if( (*g_api->key_bind)(g_plugin, 'r', on_r, "on_r secret stuff...") != 0) {
		(*g_api->log)(g_plugin, "expected to be able to bind to extension 'r'. abort...\n");
		return -1;
	}

	return 0;
}

void aug_plugin_free() {
	(*g_api->log)(g_plugin, "free\n");

	if( (*g_api->key_unbind)(g_plugin, 'r') != 0) {
		(*g_api->log)(g_plugin, "expected to be able to unbind to extension 'r'.\n");
	}
	
}