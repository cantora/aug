#include "aug_plugin.h"
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <ccan/tap/tap.h>

const char aug_plugin_name[] = "api_test";

static const struct aug_api *g_api;
static struct aug_plugin *g_plugin;

static char *g_user_data = "on_r secret stuff...";
static int g_callback_key = 0x12;
static bool g_got_callback = false;

/* if called from the main thread (a callback from aug
 * or the init and free functions), this will deadlock
 * if aug called the plugin while it had a lock on the
 * screen. if the lock is held by another plugin, then
 * this should finish eventually.
 */
static void check_screen_lock() {
	int rows, cols;

	diag("make sure screen is unlocked");
	(*g_api->term_win_dims)(g_plugin, &rows, &cols);
	pass("screen is unlocked");
}

static int test_winch() {
	sigset_t sigset;
	int winch_is_blocked;

	diag("pthread_sigmask state test");
	if(pthread_sigmask(SIG_SETMASK, NULL, &sigset) != 0) {
		fail("expected to be able to get current sigset.");
		return -1;
	}

	if( (winch_is_blocked = sigismember(&sigset, SIGWINCH) ) == -1 ) {
		fail("expected to be able test current sigset.");
		return -1;
	}
	
	ok( (winch_is_blocked == 1), "confirm that SIGWINCH is blocked" );

	return 0;
}

static void on_r(int chr, void *user) {
	diag("++++key callback++++");
	check_screen_lock();
	test_winch();
	ok( (chr == g_callback_key), "callback on key 0x%02x (got 0x%02x)", g_callback_key, chr);
	ok( (user == g_user_data), "user ptr is correct");
		
	g_got_callback = true;
	diag("----key callback----\n#");
}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	const char *testkey;
	int stack_size = -1;
	int pos = -1;

	diag("++++plugin_init++++");
	g_plugin = plugin;	
	g_api = api;

	(*g_api->log)(g_plugin, "init %s\n", aug_plugin_name);

	check_screen_lock();
	test_winch();

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
	diag("----plugin_init----\n#");

	return 0;
}

void aug_plugin_free() {

	diag("++++plugin_free++++");
	(*g_api->log)(g_plugin, "free\n");

	check_screen_lock();
	test_winch();

	ok( (g_got_callback == true) , "check to see if the key extension callback happened" );

	ok( ( (*g_api->key_unbind)(g_plugin, g_callback_key) == 0), "check to make sure we can unbind key extension");	

	diag("----plugin_free----\n#");
}