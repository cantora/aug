#include "aug_plugin.h"
#include "api_test_vars.h"
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
static WINDOW *g_pan2_dwin;
static pthread_t g_thread1, g_thread2;

/* if called from the main thread (a callback from aug
 * or the init and free functions), this will deadlock
 * if aug called the plugin while it had a lock on the
 * screen. if the lock is held by another plugin, then
 * this should finish eventually.
 */
static void check_screen_lock() {
	int size;

	diag("make sure screen is unlocked");
	(*g_api->screen_panel_size)(g_plugin, &size);
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

		/*diag("========> '%c' (0x%02x)", (*ch > 0x20 && *ch <= 0x7e)? *ch : ' ', *ch);*/
		if(*ch == '\n') {
			ok( strncmp(intern, api_test_on_r_response, CUTOFF_INTERN) == 0, 
							"check the on_r interactive input data");
			g_on_r_interaction = false;

			(*g_api->lock_screen)(g_plugin);
			ok( hide_panel(g_pan2) != ERR, "hide on_r panel");
			(*g_api->unlock_screen)(g_plugin);

			(*g_api->screen_panel_update)(g_plugin);
		}
		else {
			(*g_api->lock_screen)(g_plugin);
			waddch(g_pan2_dwin, *ch);
			wsyncup(g_pan2_dwin);
			wcursyncup(g_pan2_dwin);
			(*g_api->unlock_screen)(g_plugin);

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
	
	sleep(6);
	diag("move panel a bit");

	(*g_api->lock_screen)(g_plugin);
	ok1(move_panel(g_pan1, 10, 30) != ERR);
	(*g_api->unlock_screen)(g_plugin);

	(*g_api->screen_panel_update)(g_plugin);
	(*g_api->screen_doupdate)(g_plugin);
	diag("sleep for a while and then hide bottom panel");
	sleep(3);

	(*g_api->lock_screen)(g_plugin);
	ok1(hide_panel(g_pan1) != ERR);
	(*g_api->unlock_screen)(g_plugin);

	(*g_api->screen_panel_update)(g_plugin);
	(*g_api->screen_doupdate)(g_plugin);
	sleep(1);
	diag("----thread1----\n#");
	return NULL;
}

static void *thread2(void *user) {
	(void)(user);
	int stack_size;
	int rows, cols;
	WINDOW *pan2_win;

	diag("++++thread2++++");
	check_screen_lock();
	test_winch();

	diag("allocate a panel");
	rows = 10;
	cols = 30;
	(*g_api->screen_panel_alloc)(g_plugin, rows, cols, 10, 15, &g_pan2);
	pass("panel allocated");

	diag("there should be only 2 panels");

	todo_start("expected to fail. see below.");
	(*g_api->screen_panel_size)(g_plugin, &stack_size);
	ok1(stack_size == 2);
	todo_end();

	diag("write a message into the panel");

	(*g_api->lock_screen)(g_plugin);
	if( (pan2_win = panel_window(g_pan2) ) == NULL) {
		diag("expected to be able to access panel window. abort...");
		goto unlock;
	}
	
	g_pan2_dwin = derwin(pan2_win, rows - 3, cols - 2, 2, 1);

	if(box_and_print(pan2_win, "the ^R panel") != 0) {
		diag("box_and_print failed. abort...");
		goto unlock;
	}
	
	mvwprintw(g_pan2_dwin, 0, 0, "enter a string: ");
	(*g_api->unlock_screen)(g_plugin);

	(*g_api->screen_panel_update)(g_plugin);
	(*g_api->screen_doupdate)(g_plugin);

	g_on_r_interaction = true;
	
	while(g_on_r_interaction == true)
		usleep(10000);

	diag("----thread2----\n#");
	return NULL;
unlock:
	(*g_api->unlock_screen)(g_plugin);
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

/* screen is locked already */
void status_bar_cb(WINDOW *win, void *user) {
	static int ran_once = 0;
	int rows, cols, y, x;
	(void)(user);

	if(ran_once == 0) {
		pass("got callback for status bar window");
		ok1(win != NULL);
		getmaxyx(win, rows, cols);
		ok1(rows == 3);
		ok1(cols == COLS);
		getparyx(win, y, x);
		ok1(y == 0);
		ok1(x == 0);
		ran_once = 1;
	}

	if(win != NULL) {
		if(box_and_print(win, "status bar!") != 0)
			diag("warning: box_and_print on status window failed.");

		wsyncup(win);
		wcursyncup(win);
		if(wnoutrefresh(win) == ERR)
			diag("warning: expected to be able to refresh status window");
	}
}

void bottom_bar1_cb(WINDOW *win, void *user) {
	static int ran_once = 0;
	int rows, cols, y, x;

	(void)(user);

	if(ran_once == 0) {
		pass("got callback for bottom bar1 window");
		ok1(win != NULL);
		getmaxyx(win, rows, cols);
		ok1(rows == 4);
		ok1(cols == COLS);
		getparyx(win, y, x);
		ok1(y == LINES-5);
		ok1(x == 0);
		ran_once = 1;
	}

	if(win != NULL) {
		if(box_and_print(win, "bottom bar 1!") != 0)
			diag("warning: box_and_print on status window failed.");

		wsyncup(win);
		wcursyncup(win);
		if(wnoutrefresh(win) == ERR)
			diag("warning: expected to be able to refresh status window");
	}
}

void bottom_bar0_cb(WINDOW *win, void *user) {
	static int ran_once = 0;
	int rows, cols, y, x;

	(void)(user);

	if(ran_once == 0) {
		pass("got callback for bottom bar0 window");
		ok1(win != NULL);
		getmaxyx(win, rows, cols);
		ok1(rows == 1);
		ok1(cols == COLS);
		getparyx(win, y, x);
		ok1(y == LINES-1);
		ok1(x == 0);
		ran_once = 1;
	}

	if(win != NULL) {
		
		if(mvwprintw(win, 0, 1, "bottom bar 0") == ERR)
			diag("warning: print on window failed.");

		wsyncup(win);
		wcursyncup(win);
		if(wnoutrefresh(win) == ERR)
			diag("warning: expected to be able to refresh window");
	}
}

void left_bar0_cb(WINDOW *win, void *user) {
	static int ran_once = 0;
	int rows, cols, y, x, i, j;

	(void)(user);

	if(ran_once == 0) {
		pass("got callback for left bar0 window");
		ok1(win != NULL);
		getmaxyx(win, rows, cols);
		ok1(cols == 1);
		ok1(rows == LINES-8);
		getparyx(win, y, x);
		ok1(y == 3);
		ok1(x == 0);
		ran_once = 1;
	}

	if(win != NULL) {
		getmaxyx(win, rows, cols);		
		diag("left window: %dx%d", rows, cols);
		for(i = 0; i < rows; i++)
			for(j = 0; j < cols; j++)
				if(mvwaddch(win, i, j, '|') == ERR)
					diag("warning: print on window failed at %d/%d, %d/%d", i, rows, j, cols);

		wsyncup(win);
		wcursyncup(win);
		if(wnoutrefresh(win) == ERR)
			diag("warning: expected to be able to refresh window");
	}
}

void left_bar1_cb(WINDOW *win, void *user) {
	static int ran_once = 0;
	int rows, cols, y, x, i, j;

	(void)(user);

	if(ran_once == 0) {
		pass("got callback for left bar1 window");
		ok1(win != NULL);
		getmaxyx(win, rows, cols);
		ok1(cols == 10);
		ok1(rows == LINES-8);
		getparyx(win, y, x);
		ok1(y == 3);
		ok1(x == 1);
		ran_once = 1;
	}

	if(win != NULL) {
		getmaxyx(win, rows, cols);		
		for(i = 0; i < rows; i++)
			for(j = 0; j < cols; j++)
				if(mvwaddch(win, i, j, '*') == ERR)
					diag("warning: print on window failed at %d/%d, %d/%d", i, rows, j, cols);

		wsyncup(win);
		wcursyncup(win);
		if(wnoutrefresh(win) == ERR)
			diag("warning: expected to be able to refresh window");
	}
}

void left_bar2_cb(WINDOW *win, void *user) {
	static int ran_once = 0;
	int rows, cols, y, x, i, j;

	(void)(user);

	if(ran_once == 0) {
		pass("got callback for left bar2 window");
		ok1(win != NULL);
		getmaxyx(win, rows, cols);
		ok1(cols == 3);
		ok1(rows == LINES-8);
		getparyx(win, y, x);
		ok1(y == 3);
		ok1(x == 11);
		ran_once = 1;
	}

	if(win != NULL) {
		getmaxyx(win, rows, cols);		
		for(i = 0; i < rows; i++)
			for(j = 0; j < cols; j++)
				if(mvwaddch(win, i, j, '+') == ERR)
					diag("warning: print on window failed at %d/%d, %d/%d", i, rows, j, cols);

		wsyncup(win);
		wcursyncup(win);
		if(wnoutrefresh(win) == ERR)
			diag("warning: expected to be able to refresh window");
	}
}

void right_bar0_cb(WINDOW *win, void *user) {
	static int ran_once = 0;
	int rows, cols, y, x, i, j;

	(void)(user);

	if(ran_once == 0) {
		pass("got callback for right bar0 window");
		ok1(win != NULL);
		getmaxyx(win, rows, cols);
		ok1(cols == 5);
		ok1(rows == LINES-8);
		getparyx(win, y, x);
		ok1(y == 3);
		ok1(x == COLS-5);
		ran_once = 1;
	}

	if(win != NULL) {
		getmaxyx(win, rows, cols);		
		for(i = 0; i < rows; i++)
			for(j = 0; j < cols; j++)
				if(mvwaddch(win, i, j, '!') == ERR)
					diag("warning: print on window failed at %d/%d, %d/%d", i, rows, j, cols);

		wsyncup(win);
		wcursyncup(win);
		if(wnoutrefresh(win) == ERR)
			diag("warning: expected to be able to refresh window");
	}
}

void right_bar1_cb(WINDOW *win, void *user) {
	static int ran_once = 0;
	int rows, cols, y, x, i, j;

	(void)(user);

	if(ran_once == 0) {
		pass("got callback for right bar1 window");
		ok1(win != NULL);
		getmaxyx(win, rows, cols);
		ok1(cols == 2);
		ok1(rows == LINES-8);
		getparyx(win, y, x);
		ok1(y == 3);
		ok1(x == COLS-7);
		ran_once = 1;
	}

	if(win != NULL) {
		getmaxyx(win, rows, cols);		
		for(i = 0; i < rows; i++)
			for(j = 0; j < cols; j++)
				if(mvwaddch(win, i, j, '?') == ERR)
					diag("warning: print on window failed at %d/%d, %d/%d", i, rows, j, cols);

		wsyncup(win);
		wcursyncup(win);
		if(wnoutrefresh(win) == ERR)
			diag("warning: expected to be able to refresh window");
	}
}

void right_bar2_cb(WINDOW *win, void *user) {
	static int ran_once = 0;
	int rows, cols, y, x, i, j;

	(void)(user);

	if(ran_once == 0) {
		pass("got callback for right bar2 window");
		ok1(win != NULL);
		getmaxyx(win, rows, cols);
		ok1(cols == 2);
		ok1(rows == LINES-8);
		getparyx(win, y, x);
		ok1(y == 3);
		ok1(x == COLS-9);
		ran_once = 1;
	}

	if(win != NULL) {
		getmaxyx(win, rows, cols);		
		for(i = 0; i < rows; i++)
			for(j = 0; j < cols; j++)
				if(mvwaddch(win, i, j, '@') == ERR)
					diag("warning: print on window failed at %d/%d, %d/%d", i, rows, j, cols);

		wsyncup(win);
		wcursyncup(win);
		if(wnoutrefresh(win) == ERR)
			diag("warning: expected to be able to refresh window");
	}
}

void right_bar3_cb(WINDOW *win, void *user) {
	static int ran_once = 0;
	int rows, cols, y, x, i, j;

	(void)(user);

	if(ran_once == 0) {
		pass("got callback for right bar3 window");
		ok1(win != NULL);
		getmaxyx(win, rows, cols);
		ok1(cols == 1);
		ok1(rows == LINES-8);
		getparyx(win, y, x);
		ok1(y == 3);
		ok1(x == COLS-10);
		ran_once = 1;
	}

	if(win != NULL) {
		getmaxyx(win, rows, cols);		
		for(i = 0; i < rows; i++)
			for(j = 0; j < cols; j++)
				if(mvwaddch(win, i, j, '|') == ERR)
					diag("warning: print on window failed at %d/%d, %d/%d", i, rows, j, cols);

		wsyncup(win);
		wcursyncup(win);
		if(wnoutrefresh(win) == ERR)
			diag("warning: expected to be able to refresh window");
	}
}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	const char *testkey;
	int stack_size = -1;
	WINDOW *pan1_win;

	plan_tests(102);
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

	(*g_api->lock_screen)(g_plugin);
	if( (pan1_win = panel_window(g_pan1) ) == NULL) {
		diag("expected to be able to access panel window. abort...");
		goto unlock;
	}
	
	if(box_and_print(pan1_win, "the bottom panel") != 0) {
		diag("box_and_print failed. abort...");
		goto unlock;
	}
	(*g_api->unlock_screen)(g_plugin);

	(*g_api->screen_panel_update)(g_plugin);
	(*g_api->screen_doupdate)(g_plugin);

	diag("create status bar window");
	(*g_api->screen_win_alloc_top)(g_plugin, 3, status_bar_cb);
	(*g_api->screen_win_alloc_bot)(g_plugin, 1, bottom_bar0_cb);
	(*g_api->screen_win_alloc_bot)(g_plugin, 4, bottom_bar1_cb);
	(*g_api->screen_win_alloc_left)(g_plugin, 1, left_bar0_cb);
	(*g_api->screen_win_alloc_left)(g_plugin, 10, left_bar1_cb);
	(*g_api->screen_win_alloc_left)(g_plugin, 3, left_bar2_cb);
	(*g_api->screen_win_alloc_right)(g_plugin, 5, right_bar0_cb);
	(*g_api->screen_win_alloc_right)(g_plugin, 2, right_bar1_cb);
	(*g_api->screen_win_alloc_right)(g_plugin, 2, right_bar2_cb);
	(*g_api->screen_win_alloc_right)(g_plugin, 1, right_bar3_cb);
	
	diag("create thread for asynchronous tests");
	if(pthread_create(&g_thread1, NULL, thread1, NULL) != 0) {
		diag("expected to be able to create a thread. abort...");
		return -1;
	}

	diag("----plugin_init----\n#");

	return 0;
unlock:
	(*g_api->unlock_screen)(g_plugin);
	return -1;
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
	ok(delwin(g_pan2_dwin) != ERR, "delete derived window");

	todo_start("screen_panel_size fails because "
				"hidden panels are not traversable via "
				"panel_below/above functions, which "
				"messes up panel_stack_size()");
	(*g_api->screen_panel_size)(g_plugin, &size);
	ok1( size == 2 );
	(*g_api->screen_panel_dealloc)(g_plugin, g_pan2);
	(*g_api->screen_panel_size)(g_plugin, &size);
	ok1( size == 1 );
	(*g_api->screen_panel_dealloc)(g_plugin, g_pan1);
	(*g_api->screen_panel_size)(g_plugin, &size);
	todo_end();
	ok1( size == 0 );
	
	diag("----plugin_free----\n#");
}