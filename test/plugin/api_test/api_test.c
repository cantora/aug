#include "aug_plugin.h"
#include <string.h>
#include <stdbool.h>
#include <ccan/tap/tap.h>

const char aug_plugin_name[] = "api_test";

static const struct aug_api *g_api;
static struct aug_plugin *g_plugin;

static char *g_user_data = "on_r secret stuff...";
static int g_callback_key = 0x12;
static bool g_got_callback = false;

static void on_r(int chr, void *user) {
	ok( (chr == g_callback_key), "callback on key 0x%02x (got 0x%02x)", g_callback_key, chr);
	ok( (user == g_user_data), "user ptr is correct");
		
	g_got_callback = true;
}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	const char *testkey;
	int stack_size = -1;
	int pos = -1;

	g_plugin = plugin;	
	g_api = api;

	(*g_api->log)(g_plugin, "init %s\n", aug_plugin_name);

	diag("test for the value of testkey in the ini file");	
	if( (*g_api->conf_val)(g_plugin, aug_plugin_name, "testkey", &testkey) == 0) {
		if(strcmp(testkey, "testval") != 0) {
			fail("testkey != testval");
		}			
		else {
			pass("testkey = testval");
		}
	}
	else {
		fail("testkey not found.");
	}	

	(*g_api->stack_size)(g_plugin, &stack_size);
	ok(stack_size == 2, "test stack size function: should be 2 plugins.");

	diag("check if this plugin is at position 1 in the stack");
	if( (*g_api->stack_pos)(g_plugin, g_plugin->name, &pos) != 0) {
		fail("should have found self at a plugin stack position. abort...\n");
	}	
	else if( pos != 1) {
		fail("expected to be at stack position 1.");
	}
	else
		pass("pos = 1");

	/* register a callback for when the user types command key + 'r' */
	if( (*g_api->key_bind)(g_plugin, g_callback_key, on_r, g_user_data) != 0) {
		diag("expected to be able to bind to extension '^R'. abort...");
		return -1;
	}

	return 0;
}

void aug_plugin_free() {
	(*g_api->log)(g_plugin, "free\n");

	ok( (g_got_callback == true) , "check to see if the key extension callback happened" );

	ok( ( (*g_api->key_unbind)(g_plugin, g_callback_key) == 0), "check to make sure we can unbind key extension");	
}