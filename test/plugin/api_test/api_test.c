#include "aug_plugin.h"
#include "api_test.h"
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <ccan/tap/tap.h>
#include <ccan/array_size/array_size.h>

const char aug_plugin_name[] = "api_test";

void input_char(int *ch, aug_action *action, void *user);
void cell_update(int *row, int *col, wchar_t *wch, attr_t *attr, 
					int *color_pair, aug_action *action, void *user);
void cursor_move(int old_row, int old_col, int *new_row, int *new_col, 
					aug_action *action, void *user);
void screen_dims_change(int rows, int cols, void *user);

static const struct aug_api *g_api;
static struct aug_plugin *g_plugin;

static char *g_user_data = "on_r secret stuff...";
static struct aug_plugin_cb g_callbacks = {
	.input_char = input_char,
	.cell_update = cell_update,
	.cursor_move = cursor_move,
	.screen_dims_change = screen_dims_change
};
static int g_callback_key = 0x12;
static bool g_got_callback = false;
static bool g_got_expected_input = false;
static bool g_got_cell_update = false;
static bool g_got_cursor_move = false;
static bool g_on_r_interaction = false;
static PANEL *g_pan1, *g_pan2;
static pthread_t g_thread1, g_thread2;

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

void input_char(int *ch, aug_action *action, void *user) {
	static unsigned int total_chars = 0;
#	define CUTOFF (ARRAY_SIZE(api_test_user_input) - 1 )
	static char firstn[CUTOFF+1];
#	define CUTOFF_INTERN ( ARRAY_SIZE(api_test_on_r_response) - 1 - 1)
	static char intern[CUTOFF_INTERN];
	static unsigned int total_inter_chars = 0;
	(void)(action);

	/*diag("========> %d/%d: '%c' (0x%02x)", total_chars+1, CUTOFF, (*ch > 0x20 && *ch <= 0x7e)? *ch : ' ', *ch);*/
	if(total_chars < CUTOFF ) {
		firstn[total_chars] = *ch;
	}
	
	total_chars++;

	if(total_chars == CUTOFF ) {
		diag("++++input_char++++");
		firstn[total_chars] = '\0';	
		/*diag("user input = '%s'", firstn);*/
		if(strcmp(firstn, api_test_user_input) == 0)
			g_got_expected_input = true;

		ok(user == g_user_data, "check that user ptr is correct");
		check_screen_lock();
		test_winch();
		diag("----input_char----\n#");
	}

	if(g_on_r_interaction == true) {
		WINDOW *pan2_win;

		/*diag("========> '%c' (0x%02x)", (*ch > 0x20 && *ch <= 0x7e)? *ch : ' ', *ch);*/
		if(*ch == '\n') {
			ok( strncmp(intern, api_test_on_r_response, CUTOFF_INTERN) == 0, 
							"check the on_r interactive input data");
			g_on_r_interaction = false;			
		}
		else {
			if( (pan2_win = panel_window(g_pan2) ) == NULL) {
				diag("expected to be able to access panel window. abort...");
				return;
			}
		
			waddch(pan2_win, *ch);
			(*g_api->screen_panel_update)(g_plugin);
			(*g_api->screen_doupdate)(g_plugin);

			if(total_inter_chars < CUTOFF_INTERN)
				intern[total_inter_chars++] = *ch;				
		}

		*action = AUG_ACT_CANCEL;
	}

#undef CUTOFF	
}

void cell_update(int *row, int *col, wchar_t *wch, attr_t *attr, 
					int *color_pair, aug_action *action, void *user) {
	(void)(row);
	(void)(col);
	(void)(wch);
	(void)(attr);
	(void)(color_pair);
	(void)(action);
	static bool checked_winch_and_screen_lock = false;

	/*diag("cell_update: %d,%d", *row, *col);*/
	g_got_cell_update = true;

	if(checked_winch_and_screen_lock == false) {
		diag("++++cell_update++++");
		ok(user == g_user_data, "check that user ptr is correct");
		check_screen_lock();
		test_winch();
		checked_winch_and_screen_lock = true;
		diag("----cell_update----\n#");
	}
}

void cursor_move(int old_row, int old_col, int *new_row, int *new_col, 
					aug_action *action, void *user) {
	(void)(old_row);
	(void)(old_col);
	(void)(new_row);
	(void)(new_col);
	(void)(action);
	static bool checked_winch_and_screen_lock = false;

	/*diag("cursor_move: %d,%d", *new_row, *new_col);*/
	g_got_cursor_move = true;

	if(checked_winch_and_screen_lock == false) {
		diag("++++cursor_move++++");
		ok(user == g_user_data, "check that user ptr is correct");
		check_screen_lock();
		test_winch();
		checked_winch_and_screen_lock = true;
		diag("----cursor_move----\n#");
	}
}

void screen_dims_change(int rows, int cols, void *user) {

	diag("++++screen_dims_change++++");
	diag("change to %d,%d", rows, cols);
	ok(user == g_user_data, "check that user ptr is correct");
	check_screen_lock();
	test_winch();
	diag("----screen_dims_change----\n#");
}


static int box_and_print(WINDOW *win, const char *str) {
	if(box(win, 0, 0) == ERR) {
		diag("expected to be able modify the window. abort...");
		return -1;
	}
		
	if(mvwprintw(win, 1, 1, str) == ERR) {
		diag("expected to be able to print to the window. abort...");
		return -1;
	}

	return 0;
}

static void *thread1(void *user) {
	(void)(user);

	diag("++++thread1++++");
	check_screen_lock();
	test_winch();
	diag("----thread1----\n#");
	return NULL;
}

static void *thread2(void *user) {
	(void)(user);
	int stack_size;
	WINDOW *pan2_win;

	diag("++++thread2++++");
	check_screen_lock();
	test_winch();

	diag("allocate a panel");
	(*g_api->screen_panel_alloc)(g_plugin, 10, 30, 10, 15, &g_pan2);
	pass("panel allocated");

	diag("there should be only 2 panels");
	(*g_api->screen_panel_size)(g_plugin, &stack_size);
	ok1(stack_size == 2);

	diag("write a message into the panel");
	if( (pan2_win = panel_window(g_pan2) ) == NULL) {
		diag("expected to be able to access panel window. abort...");
		return NULL;
	}
	
	if(box_and_print(pan2_win, "the ^R panel\nenter a string:") != 0) {
		diag("box_and_print failed. abort...");
		return NULL;
	}
	(*g_api->screen_panel_update)(g_plugin);
	(*g_api->screen_doupdate)(g_plugin);

	g_on_r_interaction = true;
	
	while(g_on_r_interaction == true)
		usleep(10000);

	diag("----thread2----\n#");
	return NULL;
}

static void on_r(int chr, void *user) {
	diag("++++key callback++++");
	check_screen_lock();
	test_winch();
	ok( (chr == g_callback_key), "callback on key 0x%02x (got 0x%02x)", g_callback_key, chr);
	ok( (user == g_user_data), "user ptr is correct");
		
	g_got_callback = true;
	
	if(g_on_r_interaction != true) {
		diag("spawn thread for user interaction");
		if(pthread_create(&g_thread2, NULL, thread2, NULL) != 0) {
			diag("failed to create user interaction thread...");
			return;
		}
	}

	diag("----key callback----\n#");
}


int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	const char *testkey;
	int stack_size = -1;
	int pos = -1;
	WINDOW *pan1_win;

	plan_tests(40);
	diag("++++plugin_init++++");
	g_plugin = plugin;	
	g_api = api;

	(*g_api->log)(g_plugin, "init %s\n", aug_plugin_name);

	check_screen_lock();
	test_winch();

	g_callbacks.user = g_user_data;
	api->callbacks(g_plugin, &g_callbacks, NULL);

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

	diag("allocate a panel");
	(*g_api->screen_panel_alloc)(g_plugin, 10, 30, 5, 10, &g_pan1);
	pass("panel allocated");

	diag("there should be only 1 panel");
	(*g_api->screen_panel_size)(g_plugin, &stack_size);
	ok1(stack_size == 1);

	diag("write a message into the panel");
	if( (pan1_win = panel_window(g_pan1) ) == NULL) {
		diag("expected to be able to access panel window. abort...");
		return -1;
	}
	
	if(box_and_print(pan1_win, "the bottom panel") != 0) {
		diag("box_and_print failed. abort...");
		return -1;
	}
	(*g_api->screen_panel_update)(g_plugin);
	(*g_api->screen_doupdate)(g_plugin);

	diag("create thread for asynchronous tests");
	if(pthread_create(&g_thread1, NULL, thread1, NULL) != 0) {
		diag("expected to be able to create a thread. abort...");
		return -1;
	}

	diag("----plugin_init----\n#");

	return 0;
}

void aug_plugin_free() {
	int size;

	diag("++++plugin_free++++");
	(*g_api->log)(g_plugin, "free\n");

	check_screen_lock();
	test_winch();

	diag("join threads");
	pthread_join(g_thread2, NULL);
	pthread_join(g_thread1, NULL);
	diag("all threads finished");

	ok( (g_got_callback == true) , "check to see if the key extension callback happened" );

	ok( ( (*g_api->key_unbind)(g_plugin, g_callback_key) == 0), "check to make sure we can unbind key extension");	
	
	ok( (g_got_expected_input == true), "check to see if input callback got expected user input");
	ok( (g_got_cell_update == true), "check to see if cell_update callback got called");
	ok( (g_got_cursor_move == true), "check to see if cursor_move callback got called");

	diag("dealloc panels");
	(*g_api->screen_panel_size)(g_plugin, &size);
	ok1( size == 2 );
	(*g_api->screen_panel_dealloc)(g_plugin, g_pan2);
	(*g_api->screen_panel_size)(g_plugin, &size);
	ok1( size == 1 );
	(*g_api->screen_panel_dealloc)(g_plugin, g_pan1);
	(*g_api->screen_panel_size)(g_plugin, &size);
	ok1( size == 0 );
	
	diag("----plugin_free----\n#");
}