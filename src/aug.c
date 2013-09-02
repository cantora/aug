/* 
 * Copyright 2012 anthony cantor
 * This file is part of aug.
 *
 * aug is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * aug is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with aug.  If not, see <http://www.gnu.org/licenses/>.
 */
#if defined(__APPLE__)
/* needed for SIGWINCH definition */
#	define _DARWIN_C_SOURCE 
#endif
#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include <fcntl.h>

#include <wordexp.h>

#include "vterm.h"

#include <ccan/objset/objset.h>
#include <ccan/build_assert/build_assert.h>
#include <ccan/array_size/array_size.h>

#include "util.h"
#include "screen.h"
#include "err.h"
#include "timer.h"
#include "opt.h"
#include "term.h"
#include "tok_itr.h"
#include "aug.h"
#include "plugin_list.h"
#include "keymap.h"
#include "panel_stack.h"
#include "region_map.h"
#include "child.h"
#include "term_win.h"

static void resize_and_redraw_screen();
static void child_setup();
static void to_refresh_after_io();

static struct aug_conf g_conf; /* structure of configuration variables */
static struct aug_plugin_list g_plugin_list;
static bool g_plugins_initialized = false;
static struct aug_term g_term;
static dictionary *g_ini;	
static struct aug_keymap g_keymap;
static struct aug_child g_child;

static struct {
	AUG_LOCK_MEMBERS;
} g_screen;
static struct {
	AUG_LOCK_MEMBERS;
} g_region_map;
static struct {
	AUG_LOCK_MEMBERS;
} g_free_plugin_lock;

struct plugin_callback_pair {
	struct aug_plugin *plugin;
	void (*window_fn)(WINDOW *, void *user);
	void (*free_fn)(WINDOW *, void *user);
};

struct sigthread_desc {
	pthread_t tid;
	int signum;
	int waiting;
	void (*fn)(void);
	AUG_LOCK_MEMBERS;
};

struct sigthread_desc g_chld_thread, g_winch_thread;

static struct {
	OBJSET_MEMBERS(struct plugin_callback_pair *);
} g_edgewin_set;

static struct {
	AVL *tree;
	AUG_LOCK_MEMBERS;
} g_tchild_table;

static struct sigaction g_prev_winch_act;

struct aug_term_child {
	struct aug_child child;
	struct aug_term term;
	struct aug_terminal_win *twin;
	struct aug_term_win term_win;
	VTermScreenCallbacks cb_screen;
	struct aug_term_io_callbacks cb_term_io;
	int terminated;
};

/* MUST LOCK for API access to the following modules:
 * screen_*
 * panel_stack_*
 */
#define lock_screen() \
	do { \
		AUG_LOCK(&g_term); \
		AUG_LOCK(&g_screen); \
	} while(0)

#define unlock_screen() \
	do { \
		AUG_UNLOCK(&g_screen); \
		AUG_UNLOCK(&g_term); \
	} while(0)

#define lock_all() \
	do { \
		AUG_LOCK(&g_keymap); \
		AUG_LOCK(&g_plugin_list); \
		lock_screen(); \
	} while(0)

#define unlock_all() \
	do { \
		unlock_screen(); \
		AUG_UNLOCK(&g_plugin_list); \
		AUG_UNLOCK(&g_keymap); \
	} while(0)

/* ================= API FUNCTIONS ==================================== */

static int api_log(struct aug_plugin *plugin, const char *format, ...) {
	va_list args;
	int result;
	char s[64];
	time_t t;
	const struct tm *tm;

	s[0] = '\0';
	t = time(NULL);
	if( (tm = localtime(&t)) != NULL)
	strftime(s, ARRAY_SIZE(s), "%m.%d %H:%M:%S", tm);

	fprintf(stderr, "%s(%s): ", plugin->name, s);
	va_start(args, format);
	result = vfprintf(stderr, format, args);
	va_end(args);
	fflush(stderr);

	return result;
}

static int api_keyname_to_key(const struct aug_plugin *plugin, 
		const char *keyname, uint32_t *ch) {
	int result;
	(void)(plugin);
	
	lock_screen();
	result = screen_keyname_to_key(keyname, ch);
	unlock_screen();

	return result;
}

static void *do_unload(void *user) {
	struct aug_plugin *plugin;
	struct aug_plugin_item *i;
	int found;

	plugin = (struct aug_plugin *) user;

	/* this lock is to prevent a plugin from being
	 * freed by this call as well as the terminating
	 * free_plugins() call. */
	AUG_LOCK(&g_free_plugin_lock); 
	AUG_LOCK(&g_plugin_list);
	found = 0;
	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if( &i->plugin == plugin ) {
			found = 1;
			break;
		}
	}
	AUG_UNLOCK(&g_plugin_list);
	if(found == 0)
		goto unlock;

	fprintf(stderr, "unload plugin %s\n", i->plugin.name);
	(*i->plugin.free)();
	AUG_LOCK(&g_plugin_list);
	plugin_list_del(&g_plugin_list, i);
	AUG_UNLOCK(&g_plugin_list);

	AUG_UNLOCK(&g_free_plugin_lock);
	return NULL;
unlock:
	AUG_UNLOCK(&g_free_plugin_lock);
	return NULL;
}

static void api_unload(struct aug_plugin *plugin) {
	pthread_t tid;
	aug_detached_thread(do_unload, plugin, &tid);
}

/* for the time being, g_ini is not modified after initialization,
 * so i think it is safe to access it concurrently without any mutexing.
 */
static int api_conf_val(struct aug_plugin *plugin, const char *name, const char *key, const char **val) {
	const char *result;
	char full_key[AUG_MAX_PLUGIN_NAME_LEN + AUG_MAX_PLUGIN_KEY_LEN + 1 + 1]; /* + ':' + '\0' */ 
	size_t len;
	(void)(plugin);
	
	/* if g_ini is null then how did this plugin get loaded? */
	assert(g_ini != NULL);

	/*	
 	 *int i;
	 *for (i = 0; i < g_ini->size; i++) {
	 *	if (g_ini->key[i] == NULL)
	 *		continue;
	 *	printf("%d: %s\n", i, g_ini->key[i]);
	 *}
	 */

	if(strncmp(CONF_CONFIG_SECTION_CORE, name, sizeof(CONF_CONFIG_SECTION_CORE) ) == 0)
		return -1;
	
	strncpy(full_key, name, AUG_MAX_PLUGIN_NAME_LEN);
	full_key[AUG_MAX_PLUGIN_NAME_LEN] = '\0'; /* terminate in case name was too long */
	len = strlen(full_key);
	full_key[len++] = ':';
	strncpy(full_key + len, key, AUG_MAX_PLUGIN_KEY_LEN);
	full_key[sizeof(full_key)-1] = '\0'; /* terminate in case key was too long */

	/*fprintf(stderr, "lookup key %s for plugin %s\n", full_key, plugin->name);*/
	result = ciniparser_getstring(g_ini, full_key, NULL);

	if(result == NULL)
		return -1;

	/*fprintf(stderr, "found value: %s\n", result);*/

	*val = result;
	return 0;
}

static void api_callbacks(struct aug_plugin *plugin, const struct aug_plugin_cb *callbacks, const struct aug_plugin_cb **prev) {
	
	if(prev != NULL)
		*prev = plugin->callbacks;

	if(callbacks != NULL) {
		AUG_LOCK(&g_plugin_list);
		plugin->callbacks = callbacks;
		AUG_UNLOCK(&g_plugin_list);
	}
}

static int api_key_bind(const struct aug_plugin *plugin, uint32_t ch, 
							aug_on_key_fn on_key, void *user) {
	aug_on_key_fn ok;
	int result = 0;
	(void)(plugin);

	assert(on_key != NULL);
	ok = NULL;
	
	AUG_LOCK(&g_keymap);
	keymap_binding(&g_keymap, ch, &ok, NULL);
	if(ok != NULL) { /* this key is already bound */
		result = -1;
		goto unlock;
	}
	
	keymap_bind(&g_keymap, ch, on_key, user);

unlock:
	AUG_UNLOCK(&g_keymap);
	return result;
}

static int api_key_unbind(const struct aug_plugin *plugin, uint32_t ch) {
	int result;
	(void)(plugin);
	
	AUG_LOCK(&g_keymap);
	result = keymap_unbind(&g_keymap, ch);
	AUG_UNLOCK(&g_keymap);

	return result;
}

static void api_lock_screen(const struct aug_plugin *plugin) {
	(void)(plugin);
	lock_screen();
}

static void api_unlock_screen(const struct aug_plugin *plugin) {
	(void)(plugin);
	unlock_screen();
}

/* called by screen module from within screen_resize when an
 * old window has been destroyed for an allocated plugin window.
 * the plugin should take any necessary steps to cleanup the 
 * resources it allocated for this window.
 * screen, term, region_map will be locked when this is called.
 */
void make_win_alloc_cb_free(void *cb_pair, WINDOW *win) {
	struct plugin_callback_pair *pair;
	
	pair = (struct plugin_callback_pair *) cb_pair;
	if(pair->free_fn != NULL)
		(*pair->free_fn)(win, pair->plugin->callbacks->user);
}

/* called by screen module from within screen_resize when a new
 * window has been created for an allocated plugin window.
 * screen, term, region_map will be locked when this is called.
 */
void make_win_alloc_cb_new(void *cb_pair, WINDOW *win) {
	struct plugin_callback_pair *pair;
	
	pair = (struct plugin_callback_pair *) cb_pair;
	(*pair->window_fn)(win, pair->plugin->callbacks->user);
}

static inline void api_screen_win_alloc(int loc, struct aug_plugin *plugin, int size, 
		void (*window_cb)(WINDOW *, void *),
		void (*free_cb)(WINDOW *, void *) ) {
	struct plugin_callback_pair *pair;
	
	pair = aug_malloc( sizeof(struct plugin_callback_pair) );
	pair->plugin = plugin;
	pair->window_fn = window_cb;
	pair->free_fn = free_cb;
	
	/* need locks on screen, term, and region_map */
	AUG_LOCK(&g_region_map);
	lock_all();

	objset_add(&g_edgewin_set, pair);
	
	switch(loc) {
	case 0:
		region_map_push_top((void *) pair, size);
		break;
	case 1:
		region_map_push_bot((void *) pair, size);
		break;
	case 2:
		region_map_push_left((void *) pair, size);
		break;
	case 3:
		region_map_push_right((void *) pair, size);
		break;
	default:
		err_exit(0, "invalid value for loc");
	}
	resize_and_redraw_screen();

	unlock_all();
	AUG_UNLOCK(&g_region_map);
}

static void api_screen_win_alloc_top(struct aug_plugin *plugin, int nlines, 
		void (*window_cb)(WINDOW *, void *),
		void (*free_cb)(WINDOW *, void *) ) {
	api_screen_win_alloc(0, plugin, nlines, window_cb, free_cb);
}

static void api_screen_win_alloc_bot(struct aug_plugin *plugin, int nlines, 
		void (*window_cb)(WINDOW *, void *),
		void (*free_cb)(WINDOW *, void *) ) {
	api_screen_win_alloc(1, plugin, nlines, window_cb, free_cb);
}

static void api_screen_win_alloc_left(struct aug_plugin *plugin, int ncols, 
		void (*window_cb)(WINDOW *, void *),
		void (*free_cb)(WINDOW *, void *) ) {
	api_screen_win_alloc(2, plugin, ncols, window_cb, free_cb);
}

static void api_screen_win_alloc_right(struct aug_plugin *plugin, int ncols, 
		void (*window_cb)(WINDOW *, void *),
		void (*free_cb)(WINDOW *, void *) ) {
	api_screen_win_alloc(3, plugin, ncols, window_cb, free_cb);
}

static int api_screen_win_dealloc(struct aug_plugin *plugin, \
				void (*window_cb)(WINDOW *, void *)) {
	struct objset_iter i;
	struct plugin_callback_pair *pair;
	int status;

	status = -1;
	/* need locks on screen, term, and region_map */
	AUG_LOCK(&g_region_map);
	lock_all();

	/* find the edgewin */
	for(pair = objset_first(&g_edgewin_set, &i); pair != NULL; 
			pair = objset_next(&g_edgewin_set, &i) ) {
		if(plugin == pair->plugin && window_cb == pair->window_fn)
			break;
	}

	if(pair != NULL) {
		/* delete it from the region map. the next time
		 * the screen is re-assessed all window regions will be
		 * freed (with provided _free callbacks being called) 
		 * and then reallocated. since this particular region
		 * will have been deleted from the region map it will
		 * not be reallocated space and thus will have been freed
		 * for good. */
		if(region_map_delete( (void *) pair) != 0)
			err_exit(0, "expected pair to exist in region map!");

		objset_del(&g_edgewin_set, pair);
		/* re-assess what the screen should look like */
		resize_and_redraw_screen();
		free(pair);
		status = 0;
	}

	unlock_all();
	AUG_UNLOCK(&g_region_map);

	return status;
}

static void api_screen_panel_alloc(struct aug_plugin *plugin, int nlines, int ncols, 
								int begin_y, int begin_x, PANEL **panel) {
	lock_screen();

	if(nlines < 1 || ncols < 1) {
		screen_dims(&nlines, &ncols);
	}		
	panel_stack_push(plugin, nlines, ncols, begin_y, begin_x);
	panel_stack_top(panel);

	unlock_screen();	
}

static void api_screen_panel_dealloc(struct aug_plugin *plugin, PANEL *panel) {
	(void)(plugin);

	lock_screen();
	panel_stack_rm(panel);
	unlock_screen();	
}

static void api_screen_panel_size(struct aug_plugin *plugin, int *size) {
	(void)(plugin);

	lock_screen();
	panel_stack_size(size);
	unlock_screen();	
}

static void api_screen_panel_update(struct aug_plugin *plugin) {
	(void)(plugin);

	panel_stack_update();
}

static void api_screen_doupdate(struct aug_plugin *plugin) {
	(void)(plugin);

	screen_doupdate();
}

/* no locks engaged here because this API call is meant to be used
 * inside a callback our within calls to api_lock_screen and 
 * api_unlock_screen */
static void api_primary_term_damage(const struct aug_plugin *plugin, 
			size_t col_start, size_t col_end, size_t row_start, size_t row_end) {
	(void)(plugin);
	screen_defer_damage(col_start, col_end, row_start, row_end);
}

static int terminal_cb_damage(VTermRect rect, void *user) {
	struct aug_term_child *tchild;

	tchild = (struct aug_term_child *) user;
	return term_win_damage(&tchild->term_win, rect, screen_color_on());
}

static int terminal_cb_movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user) {
	struct aug_term_child *tchild;
	(void)(visible);

	tchild = (struct aug_term_child *) user;

	return term_win_movecursor(&tchild->term_win, pos, oldpos, screen_color_on());
}

static void terminal_cb_refresh(void *user) {
	struct aug_term_child *tchild;

	tchild = (struct aug_term_child *) user;
	term_win_refresh(&tchild->term_win, screen_color_on());
}

static void terminal_run_lock(void *user) {
	struct aug_term_child *tchild;

	tchild = (struct aug_term_child *) user;
	lock_all();	
	
	/* update terminal window with new curses window
	 * if it has changed */
	if(tchild->twin->win != tchild->term_win.win) {
		term_win_resize(&tchild->term_win, tchild->twin->win);
	}
}

static void terminal_run_unlock(void *user) {
	(void)(user);
	unlock_all();	
}

static void api_terminal_new(struct aug_plugin *plugin, struct aug_terminal_win *twin,
								char *const *argv, void **terminal) {
	struct aug_term_child *tchild, *old_tchild;
	int fd;
	(void)(plugin);

	AUG_LOCK(&g_tchild_table);
	lock_all();
	
	tchild = aug_malloc( sizeof(struct aug_term_child) );
	tchild->twin = twin;

	term_init(&tchild->term, 1, 1);
	tchild->terminated = 0;
	
	term_win_init(&tchild->term_win, twin->win);
	term_win_set_term(&tchild->term_win, &tchild->term);

	memset(&tchild->cb_screen, 0, sizeof(VTermScreenCallbacks) );
	tchild->cb_screen.damage 		= terminal_cb_damage;
	tchild->cb_screen.movecursor 	= terminal_cb_movecursor;
	tchild->cb_screen.bell 			= screen_bell;
	tchild->cb_screen.settermprop	= screen_settermprop;
	
	tchild->cb_term_io.refresh		= terminal_cb_refresh;
	term_set_callbacks(&tchild->term, &tchild->cb_screen, &tchild->cb_term_io, tchild);

	/* very strange bug: for some reason the select call
	 * during child_io_loop will not receive an EOF
	 * when the child process is killed unless the parent
	 * and child have shared a file descriptor at some point.
	 * i have no idea why, but opening up dev/null and closing
	 * it here in the parent after the child is created
	 * fixes it... */
	if( (fd = open("/dev/null", O_RDONLY) ) == -1)
		err_exit(errno, "failed to open dev/null");

	child_init(
		&tchild->child, 
		&tchild->term,
		argv,
		child_setup,
		to_refresh_after_io,
		terminal_run_lock,
		terminal_run_unlock,
		NULL,
		tchild
	);
	if(close(fd) != 0)
		err_exit(errno, "failed to close file descriptor");

	fprintf(stderr, "started child at pid %d\n", tchild->child.pid);

	*terminal = (void *) tchild;

	/* disregard the warning, void * is bigger or equal to pid_t */
	BUILD_ASSERT( sizeof(void *) >= sizeof(pid_t) );
	old_tchild = avl_lookup(g_tchild_table.tree, (void *) tchild->child.pid);
	if(old_tchild != NULL) {
		if(old_tchild->terminated == 0) {
			fprintf(
				stderr, 
				"warning: reusing pid %d before previous child \
					at that pid terminated", 
				tchild->child.pid
			);
			old_tchild->terminated = 1;
		}		
	}

	BUILD_ASSERT( sizeof(void *) >= sizeof(pid_t) );
	avl_insert(g_tchild_table.tree, (void *) tchild->child.pid, tchild);

	unlock_all();
	AUG_UNLOCK(&g_tchild_table);
}

static void api_terminal_delete(struct aug_plugin *plugin, void *terminal) {
	struct aug_term_child *tchild;
	(void)(plugin);
	
	AUG_LOCK(&g_tchild_table);
	lock_all();

	tchild = (struct aug_term_child *) terminal;
	BUILD_ASSERT( sizeof(void *) >= sizeof(pid_t) );
	if(avl_remove(g_tchild_table.tree, (void *) tchild->child.pid) != true) {
#ifdef AUG_DEBUG
		err_exit(
			0, 
			"error: could not locate terminal child at pid %d", 
			tchild->child.pid
		);
#else
		fprintf(
			stderr, 
			"warning: could not locate terminal child at pid %d", 
			tchild->child.pid
		);
#endif
	}
	
	child_free(&tchild->child);	
	term_win_free(&tchild->term_win);
	term_free(&tchild->term);
	free(tchild);

	unlock_all();
	AUG_UNLOCK(&g_tchild_table);
}

static pid_t api_terminal_pid(struct aug_plugin *plugin, const void *terminal) {
	const struct aug_term_child *tchild;
	(void)(plugin);

	tchild = (const struct aug_term_child *) terminal;
	
	return tchild->child.pid;
}

static void api_terminal_run(struct aug_plugin *plugin, void *terminal) {
	struct aug_term_child *tchild;

	(void)(plugin);

	tchild = (struct aug_term_child *) terminal;

	child_lock(&tchild->child);
	/* resources will be unlocked in the function */
	child_io_loop(
		&tchild->child,
		-1,
		NULL /* input to terminal is written asynchronously */
	);
	/* resources are unlocked at this point */
}

static int api_terminal_terminated(struct aug_plugin *plugin, const void *terminal) {
	const struct aug_term_child *tchild;
	(void)(plugin);

	tchild = (const struct aug_term_child *) terminal;
	
	return (tchild->terminated != 0);
}

static int terminal_push_data(struct aug_term *term, 
		const uint32_t *data, int amt) {
	int i;

	for(i = 0; i < amt; i++) {
		if(term_can_push_chars(term) < 1)
			break;
		if(term_push_char(term, data[i]) != 0)
			err_exit(0, "expected term to be able to push a character");
	}

	return i;
}

static size_t terminal_push_char_data(struct aug_term *term, 
		const char *data, int amt) {
	int i;

	for(i = 0; i < amt; i++) {
		if(term_can_push_chars(term) < 1)
			break;
		if(term_push_char(term, (uint32_t) data[i]) != 0)
			err_exit(0, "expected term to be able to push a character");
	}

	return i;
}

#define DO_IF_TCHILD_EXISTS_VAR() \
	pid_t pid

#define DO_IF_TCHILD_EXISTS_PRE(tchild_ptr) \
	child_lock(&(tchild_ptr)->child); \
	pid = (tchild_ptr)->child.pid; \
	child_unlock(&(tchild_ptr)->child); \
	/* lock the table to prevent this child from \
	 * being deleted while we are using it */ \
	AUG_LOCK(&g_tchild_table); \
	/* its possible this child got deleted while we  \
	 * were waiting for tchild_table to unlock. test whether \
	 * we can find the child in the table */ \
	if(avl_lookup(g_tchild_table.tree, (void *) pid) == NULL) \
		goto table_unlock; \
	child_lock(&(tchild_ptr)->child)


#define DO_IF_TCHILD_EXISTS_SUF(tchild_ptr, result) \
	child_unlock(&(tchild_ptr)->child); \
table_unlock: \
	AUG_UNLOCK(&g_tchild_table); \
	return result

static size_t terminal_input(struct aug_plugin *plugin, void *terminal, 
		const void *data, int is_char_data, int n) {
	size_t amt;	
	struct aug_term_child *tchild;
	DO_IF_TCHILD_EXISTS_VAR();

	(void)(plugin);

	tchild = (struct aug_term_child *) terminal;
	amt = 0;

	DO_IF_TCHILD_EXISTS_PRE(tchild);
	/* only if tchild still exists after locking */
		if(is_char_data != 0)
			amt = terminal_push_char_data(tchild->child.term, data, n);
		else
			amt = terminal_push_data(tchild->child.term, data, n);
	
		if(amt > 0) {
			child_process_term_output(&tchild->child);
			child_refresh(&tchild->child);
			child_got_input(&tchild->child);
		}
	/* end */
	DO_IF_TCHILD_EXISTS_SUF(tchild, amt); /* returns amt */
}

static size_t api_terminal_input(struct aug_plugin *plugin, void *terminal, 
		const uint32_t *data, int n) {
	return terminal_input(plugin, terminal, data, 0, n);
}

static size_t api_terminal_input_chars(struct aug_plugin *plugin, void *terminal, 
		const char *data, int n) {
	return terminal_input(plugin, terminal, data, 1, n);
}

static void api_terminal_refresh(struct aug_plugin *plugin, void *terminal) {
	struct aug_term_child *tchild;
	DO_IF_TCHILD_EXISTS_VAR();
	(void)(plugin);

	tchild = (struct aug_term_child *) terminal;

	DO_IF_TCHILD_EXISTS_PRE(tchild);
	/* only executes if tchild still exists after locking */
		child_refresh(&tchild->child);
	/* end */
	DO_IF_TCHILD_EXISTS_SUF(tchild,); /* returns nothing */
}

static size_t primary_input(struct aug_plugin *plugin, const void *data, 
			int is_char_data, int n) {
	size_t amt;
	(void)(plugin);

	/* this will invoke the configured lock callback,
	 * so theres no need to explicity lock anything else here.
	 * (see main() ). */
	child_lock(&g_child);

	if(is_char_data != 0)
		amt = terminal_push_char_data(g_child.term, data, n);
	else
		amt = terminal_push_data(g_child.term, data, n);

	if(amt > 0) {
		child_process_term_output(&g_child);
		child_refresh(&g_child);
		child_got_input(&g_child);
	}

	child_unlock(&g_child);
	return amt;
}

static size_t api_primary_input(struct aug_plugin *plugin,
		const uint32_t *data, int n) {
	return primary_input(plugin, data, 0, n);
}

static size_t api_primary_input_chars(struct aug_plugin *plugin,
		const char *data, int n) {
	return primary_input(plugin, data, 1, n);
}

static void api_primary_refresh(struct aug_plugin *plugin) {
	(void)(plugin);

	child_lock(&g_child);
	child_refresh(&g_child);
	child_unlock(&g_child);
}

/* =================== end API functions ==================== */

/* ================= term callbacks for API =========================== */

int aug_cell_update(int rows, int cols, int *row, int *col,
		wchar_t *wch, attr_t *attr, int *color_pair) {
	struct aug_plugin_item *i;
	aug_action action;

	if(g_plugins_initialized != true)
		return 0;

	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if(i->plugin.callbacks == NULL || i->plugin.callbacks->cell_update == NULL)
			continue;

		action = AUG_ACT_OK;
		(*(i->plugin.callbacks->cell_update))(
			rows, cols, row, col, 
			wch, attr, color_pair,
			&action, i->plugin.callbacks->user
		);

		if(action == AUG_ACT_CANCEL) /* plugin wants to filter this cell update */
			return -1;
	}
	
	return 0;
}

int aug_pre_scroll(int rows, int cols, int direction) {
	struct aug_plugin_item *i;
	aug_action action;

	if(g_plugins_initialized != true)
		return 0;

	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if(i->plugin.callbacks == NULL || i->plugin.callbacks->pre_scroll == NULL)
			continue;

		action = AUG_ACT_OK;
		(*(i->plugin.callbacks->pre_scroll))(
			rows, cols, direction,
			&action, i->plugin.callbacks->user
		);

		/* plugin wants to prevent scrolling, and cause a complete redraw of the screen */
		if(action == AUG_ACT_CANCEL) 
			return -1;
	}
	
	return 0;
}

int aug_post_scroll(int rows, int cols, int direction) {
	struct aug_plugin_item *i;
	aug_action action;

	if(g_plugins_initialized != true)
		return 0;

	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if(i->plugin.callbacks == NULL || i->plugin.callbacks->post_scroll == NULL)
			continue;

		action = AUG_ACT_OK;
		(*(i->plugin.callbacks->post_scroll))(
			rows, cols, direction,
			&action, i->plugin.callbacks->user
		);

		if(action == AUG_ACT_CANCEL) /* plugin wants to filter this post scroll event */
			return -1;
	}
	
	return 0;
}

int aug_cursor_move(int rows, int cols, int old_row, int old_col, 
		int *new_row, int *new_col) {
	struct aug_plugin_item *i;
	aug_action action;

	if(g_plugins_initialized != true)
		return 0;
		
	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if(i->plugin.callbacks == NULL || i->plugin.callbacks->cursor_move == NULL)
			continue;

		action = AUG_ACT_OK;	
		(*(i->plugin.callbacks->cursor_move))(
			rows, cols, old_row, 
			old_col, new_row, new_col,
			&action, i->plugin.callbacks->user
		);

		if(action == AUG_ACT_CANCEL) /* plugin wants to filter this cursor move */
			return -1;
	}

	return 0;
}

/* this function isnt used outside this file, unlike the other callbacks */
static void aug_screen_dims_change(int rows, int cols) {
	struct aug_plugin_item *i;

	if(g_plugins_initialized != true)
		return;
	
	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if(i->plugin.callbacks == NULL || i->plugin.callbacks->screen_dims_change == NULL)
			continue;
		
		(*(i->plugin.callbacks->screen_dims_change))(rows, cols, i->plugin.callbacks->user);
	}
}

void aug_primary_term_dims_change(int rows, int cols) {
	struct aug_plugin_item *i;

	if(g_plugins_initialized != true) {
		fprintf(stderr, "primary dims change cb: plugins not initialized\n");
		return;
	}
	
	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if(i->plugin.callbacks == NULL || i->plugin.callbacks->primary_term_dims_change == NULL)
			continue;
		
		(*(i->plugin.callbacks->primary_term_dims_change))(rows, cols, i->plugin.callbacks->user);
	}
}

/* == end callbacks == */

static void resize_and_redraw_screen() {
	screen_clear();
	screen_resize();
	screen_redraw_term_win();
	panel_stack_update();
	screen_doupdate();
}

static void change_sigs(int how) {
	sigset_t sigset;
	int s;

	if(sigemptyset(&sigset) != 0) 
		err_exit(errno, "sigemptyset failed"); 
	if(sigaddset(&sigset, SIGWINCH) != 0) 
		err_exit(errno, "sigaddset failed");
	if(sigaddset(&sigset, SIGCHLD) != 0) 
		err_exit(errno, "sigaddset failed");

	/* im pretty sure we arent supposed to touch
	 * sigprocmask in multithreaded context.
	 *if(sigprocmask(how, &sigset, NULL) != 0)
	 *	err_exit(errno, "sigprocmask failed");
	 */

	if((s = pthread_sigmask(how, &sigset, NULL)) != 0)
		err_exit(s, "pthread_sigmask failed");
}

static inline void block_sigs() {
	change_sigs(SIG_BLOCK);
}

static inline void unblock_sigs() {
	change_sigs(SIG_UNBLOCK);
}

/* signal strategy: as is generally recommended, all threads 
 * (including the main thread) will block all relevant signals
 * (in our case just CHLD and WINCH) and the signal handling 
 * shall be done with a dedicated thread which invokes the
 * sigwait function. thus plugins should never unblock 
 * CHLD or WINCH and handling other signals may be a bad idea
 * too.
 */
static void handler_winch() {
	int rows, cols;

	fprintf(stderr, "handler_winch: enter\n");
	AUG_LOCK(&g_region_map);
	lock_all(); /* need locks on screen, term, and region_map */
	
	fprintf(stderr, "handler_winch: locked all\n");

	vterm_screen_flush_damage(vterm_obtain_screen(g_term.vt) );

	/* in case curses installed a winch handler */
	if(g_prev_winch_act.sa_handler != NULL) {
		fprintf(stderr, "call previous winch handler\n");
		(*g_prev_winch_act.sa_handler)(SIGWINCH);
	}

	/* tell the screen manager to resize to whatever the new
	 * size is (curses knows already). the screen manager
	 * resizes all its windows, then tells the terminal window
	 * manager and plugins about the resize */
	screen_resize();
	screen_dims(&rows, &cols);
	/* plugin callbacks */
	aug_screen_dims_change(rows, cols);

	unlock_all();
	AUG_UNLOCK(&g_region_map);
	fprintf(stderr, "handler_winch: unlocked all\n");

	fprintf(stderr, "handler_winch: exit\n");
}

/* handler for SIGCHLD. */
static void handler_chld() {
	pid_t pid;
	int status;
	struct aug_term_child *tchild;

	fprintf(stderr, "handler_chld: enter\n");

	AUG_LOCK(&g_tchild_table);
	while( (pid = waitpid(-1, &status, WNOHANG)) > 0) {
		if(!WIFEXITED(status) && !WIFSIGNALED(status) ) 
			continue;
		
		if(pid == g_child.pid) {
			fprintf(stderr, "reaped primary child at pid %d\n", pid);
			continue;
		}

		BUILD_ASSERT( sizeof(void *) >= sizeof(pid_t) );
		tchild = avl_lookup(g_tchild_table.tree, (void *) pid);
		if(tchild == NULL) {
			fprintf(stderr, "warning: reaped unknown child at pid %d\n", pid);
			continue;
		}
		fprintf(stderr, "reaped child at pid %d\n", pid);
		tchild->terminated = 1;	
	}
	AUG_UNLOCK(&g_tchild_table);

	if(pid != 0 && errno != ECHILD) {
		err_exit(errno, "waitpid caused an error");
	}

	fprintf(stderr, "handler_chld: exit\n");
}

static void *sig_thread(void *user) {
	struct sigthread_desc *desc;
	int s, signum;
	sigset_t set;

	desc = (struct sigthread_desc *) user;
	if(sigemptyset(&set) != 0)
		err_exit(errno, "sigemptyset failed");
	if(sigaddset(&set, desc->signum) != 0)
		err_exit(errno, "sigaddset failed");

	while(1) {
		AUG_LOCK(desc);
		desc->waiting = 1;
		AUG_UNLOCK(desc);

		/* we use sigwait instead of sigtimedwait here because
		 * it is not available on some versions of OSX and 
		 * some versions of openbsd. if we had sigtimedwait
		 * we wouldnt need to cancel this thread when exiting. */
		s = sigwait(&set, &signum);
		/* we only cancel when waiting == 1, so we dont have to 
		 * deal with pthread_cleanup* headaches */
		pthread_testcancel();

		AUG_LOCK(desc);
		desc->waiting = 0;
		AUG_UNLOCK(desc);
		pthread_testcancel();

		if(s != 0) {
			if(s != EAGAIN)
				err_exit(s, "sigwait on "
						"signal %d failed", desc->signum);
		}
		else
			(*desc->fn)();
	}

	return NULL;
}

static void start_sig_threads() {
	int s;

	g_chld_thread.signum = SIGCHLD;
	g_chld_thread.waiting = 0;
	g_chld_thread.fn = handler_chld;
	AUG_LOCK_INIT(&g_chld_thread);

	g_winch_thread.signum = SIGWINCH;
	g_winch_thread.waiting = 0;
	g_winch_thread.fn = handler_winch;
	AUG_LOCK_INIT(&g_winch_thread);

	s = pthread_create(&g_chld_thread.tid, NULL, sig_thread, &g_chld_thread);
	if(s != 0)
		err_exit(s, "failed to create SIGCHLD thread");

	s = pthread_create(&g_winch_thread.tid, NULL, sig_thread, &g_winch_thread);
	if(s != 0)
		err_exit(s, "failed to create SIGWINCH thread");

}

#define AUG_STOP_SIG_THREAD(s, desc_ptr) \
	do { \
		while(1) { \
			AUG_LOCK(desc_ptr); \
			if((desc_ptr)->waiting != 0) { \
				break; \
			} \
			AUG_UNLOCK(desc_ptr); \
			usleep(100000); \
		} \
		if((s = pthread_cancel((desc_ptr)->tid)) != 0) \
			err_exit(s, "failed to cancel signal thread"); \
		AUG_UNLOCK(desc_ptr); \
		if((s = pthread_join((desc_ptr)->tid, NULL)) != 0) \
			err_warn(s, "failed to join signal thread"); \
		AUG_LOCK_FREE(desc_ptr); \
	} while(0)

void stop_chld_thread() {
	int s;
	AUG_STOP_SIG_THREAD(s, &g_chld_thread);
}

void stop_winch_thread() {
	int s;
	AUG_STOP_SIG_THREAD(s, &g_winch_thread);
}

/* all resources should be locked during this function */
static void push_key(struct aug_term *term, uint32_t ch) {
	struct aug_plugin_item *i;
	aug_action action;
	
	/*fprintf(stderr, "push_key: 0x%04x\n", ch);*/
	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if(i->plugin.callbacks == NULL || i->plugin.callbacks->input_char == NULL)
			continue;
		
		action = AUG_ACT_OK;
		(*(i->plugin.callbacks->input_char))(&ch, &action, i->plugin.callbacks->user);

		if(action == AUG_ACT_CANCEL) /* plugin wants to filter this character */
			return;
	}

	term_push_char(term, ch);
}

static int process_keys(struct aug_term *term, int fd_input, void *user) {
	uint32_t ch;
	static bool command_key = false;
	aug_on_key_fn command_fn;
	void *key_user;
	
	(void)(fd_input);
	(void)(user);

	/* we need at least two spots in the buffer because in 'pass through' 
	 * command prefix mode we will send both the command key and the following
	 * key at the same time after finding a non-command */
	while(term_can_push_chars(term) > 1 && screen_getch(&ch) == 0 ) {
		if(command_key == true) { /* treat *ch* as a command extension */
			fprintf(stderr, "check for command extension 0x%02x\n", ch);
			keymap_binding(&g_keymap, ch, &command_fn, &key_user);
			if(command_fn == NULL) { /* this is not a bound key */
				push_key(term, g_conf.cmd_key);
				push_key(term, ch);
			}
			else { /* invoke the command */
				/* note: sigs should still be blocked */
				(*command_fn)(ch, key_user);
			}
			command_key = false;
		}
		else if(ch == g_conf.cmd_key) {
			command_key = true;	
		}
		else {
			push_key(term, ch);
		}
	}

	return 0;
}

static void err_exit_cleanup(int error) {
	(void)(error);

	screen_cleanup();
}

static int init_conf(int argc, char *argv[]) {
	const char *debug_file;
	bool have_config = false;
	int config_err = 0;
	const char *errmsg;
	wordexp_t exp;
	int exp_status;
	
	conf_init(&g_conf);
	if(opt_parse(argc, argv, &g_conf) != 0) {
		switch(errno) {
		case OPT_ERR_HELP:
			opt_print_help(stderr, argc, (const char *const *) argv);
			return 1;
			
		case OPT_ERR_USAGE:
			opt_print_usage(stderr, argc, (const char *const *) argv);	
			break;
		
		default:
			fprintf(stderr, "%s\n", opt_err_msg);
			opt_print_usage(stderr, argc, (const char *const *) argv);
			fputc('\n', stderr);
		}	
		
		return -1;
	}

	memset(&exp, 0, sizeof(exp) );
	if( (exp_status = wordexp(g_conf.conf_file, &exp, WRDE_NOCMD)) != 0 ) {
		switch(exp_status) {
		case WRDE_BADCHAR:
			fprintf(stderr, "bad character in config file path\n");
			break;
		case WRDE_CMDSUB:
			fprintf(stderr, "command substitution in config file path\n");
			break;
		case WRDE_SYNTAX:
			fprintf(stderr, "syntax error in config file path\n");
			break;
		default:
			err_exit(0, "unknown error during configuration file path expansion");
		}
		opt_print_usage(stderr, argc, (const char *const *) argv);
		fputc('\n', stderr);
		return -1;
	}

	if(exp.we_wordc != 1) {
		if(exp.we_wordc == 0)
			fprintf(stderr, "config file path did not expand to any words\n");
		else
			fprintf(stderr, "config file path expanded to multiple words\n");

		opt_print_usage(stderr, argc, (const char *const *) argv);
		fputc('\n', stderr);
		wordfree(&exp);
		return -1;
	}

	if(access(exp.we_wordv[0], R_OK) == 0) {
		g_ini = ciniparser_load(exp.we_wordv[0]);
		if(g_ini != NULL) {
			have_config = true;
			conf_merge_ini(&g_conf, g_ini);
		}
	}
	else
		config_err = errno;

	wordfree(&exp);

	if(g_conf.debug_file != NULL)
		debug_file = g_conf.debug_file;
	else
		debug_file = "/dev/null";
		
	if(freopen(debug_file, "w", stderr) == NULL) {
		err_exit(errno, "redirect stderr");
	}		
	
	if(have_config == false) {
		fprintf(stderr, "unable to access config file at %s: %s\n", 
					g_conf.conf_file, (config_err == 0? "ini parse error" : strerror(config_err) ) );
	}

	if(conf_set_derived_vars(&g_conf, &errmsg) != 0) {
		fprintf(stderr, "%s\n", errmsg);
		return -1;
	}

	return 0;
}

static void load_plugins() {
	int i, nsec;
	char *secname;
	const char *err;
	size_t seclen;
	char path[2048];
	struct aug_tok_itr tok_itr;
	size_t core_seclen;

	core_seclen = sizeof(CONF_CONFIG_SECTION_CORE)-1;

	fprintf(stderr, "load plugins...\n");
	if(g_ini == NULL)
		return;

	if( (nsec = ciniparser_getnsec(g_ini) ) < 1)
		return;

	secname = NULL;
	
	for(i = 1; i <= nsec; i++) {
		secname = ciniparser_getsecname(g_ini, i);
		assert(secname != NULL);

		seclen = strlen(secname);
		/*fprintf(stderr, "section %d: %s\n", i, secname);*/
		if( seclen == core_seclen && strncmp(secname, CONF_CONFIG_SECTION_CORE, core_seclen) == 0 )
			continue;

		if(seclen > AUG_MAX_PLUGIN_NAME_LEN) {  /* seems a bit excessive... */
			fprintf(stderr, "plugin section name exceeds maximum plugin name length of %d\n", AUG_MAX_PLUGIN_NAME_LEN);
			continue;
		}

		err = "could not find plugin in search path";
		/* this is a plugin section so we want to 
		 * go looking for it in the plugin_path */
		fprintf(stderr, "search for plugin: %s\n", secname);
		TOK_ITR_FOREACH(path, (1024-seclen-3-1), g_conf.plugin_path, ':', &tok_itr) {
			wordexp_t exp;
			int exp_status, found;
			size_t j;

			found = 0;
			memset(&exp, 0, sizeof(exp));

			if( (exp_status = wordexp(path, &exp, WRDE_NOCMD)) != 0 ) {
				switch(exp_status) {
				case WRDE_BADCHAR:
					fprintf(stderr, "bad character in search path %s\n", path);
					break;
				case WRDE_CMDSUB:
					fprintf(stderr, "command substitution in search path %s\n", path);
					break;
				case WRDE_SYNTAX:
					fprintf(stderr, "syntax error in search path %s\n", path);
					break;
				default:
					err_exit(0, "unknown error during configuration file path expansion");
				}

				/* assuming here that we dont need to wordfree(exp) if
				 * wordexp failed */
				continue;
			}
			if(exp.we_wordc < 1) {
				fprintf(stderr, "search path did not expand to any words: %s\n", path);
				goto next;
			}
			
			for(j = 0; j < exp.we_wordc; j++) {
				size_t len;

				strncpy(
					path, 
					exp.we_wordv[j], 
				    /*              '/'                         '.so' '\0' */
					(sizeof(path) - (1 + AUG_MAX_PLUGIN_NAME_LEN + 3) - 1)
				);
				path[sizeof(path)-1] = '\0';
				fprintf(stderr, "\tsearching in %s\n", path);

				len = strlen(path);
				if(path[len-1] != '/') {
					path[len++] = '/';
					path[len] = '\0';
				}
					
				strncat(path, secname, AUG_MAX_PLUGIN_NAME_LEN);
				strcat(path, ".so");
	
				if(access(path, R_OK) == 0) {
					/* load the plugin */
					err = NULL;
					plugin_list_push(&g_plugin_list, path, secname, seclen, &err);
					if(err == NULL)
						fprintf(stderr, "\tloaded %s.so\n", secname);

					found = 1;	
					/* escapes this for loop, continues to wordfree */
					break; 
				}
			} /* for each expanded path */

		next:
			wordfree(&exp);
			if(found != 0)
				break;
		} /* FOREACH */
		
		if(err != NULL)	
			fprintf(stderr, "\terror loading plugin %s: %s\n", secname, err);

	} /* for each section */

}

static void child_setup() {
	const char *child_term;
	const char *env_term;

	env_term = getenv("TERM"); 
	/* set terminal profile for CHILD to supplied --term arg if exists 
	 * otherwise make sure to reset the TERM variable to the initial
	 * environment, even if it was null.
	 */
	child_term = NULL;
	if(g_conf.term != NULL) 
		child_term = g_conf.term;
	else if(env_term != NULL) 
		child_term = env_term;
		
	if(child_term != NULL) {
		if(setenv("TERM", child_term, 1) != 0)
			err_exit(errno, "error setting environment variable: %s", child_term);
	}
	else {
		if(unsetenv("TERM") != 0)
			err_exit(errno, "error unsetting environment variable: TERM");
	}

	/*fprintf(stderr, "child started!\n");*/
	unblock_sigs();
}

static void init_plugins(struct aug_api *api) {
	struct aug_plugin_item *i, *next;

	plugin_list_init(&g_plugin_list);
	load_plugins();
	api->log = api_log;
	api->keyname_to_key = api_keyname_to_key;
	api->unload = api_unload;
	api->conf_val = api_conf_val;
	api->callbacks = api_callbacks;
	api->key_bind = api_key_bind;
	api->key_unbind = api_key_unbind;

	api->lock_screen = api_lock_screen;
	api->unlock_screen = api_unlock_screen;

	api->screen_win_alloc_top = api_screen_win_alloc_top;
	api->screen_win_alloc_bot = api_screen_win_alloc_bot;
	api->screen_win_alloc_left = api_screen_win_alloc_left;
	api->screen_win_alloc_right = api_screen_win_alloc_right;
	api->screen_win_dealloc = api_screen_win_dealloc;

	api->screen_panel_alloc = api_screen_panel_alloc;
	api->screen_panel_dealloc = api_screen_panel_dealloc;
	api->screen_panel_size = api_screen_panel_size;
	api->screen_panel_update = api_screen_panel_update;
	api->screen_doupdate = api_screen_doupdate;
	api->primary_term_damage = api_primary_term_damage;
	api->terminal_new = api_terminal_new;
	api->terminal_delete = api_terminal_delete;
	api->terminal_run = api_terminal_run;
	api->terminal_pid = api_terminal_pid;
	api->terminal_terminated = api_terminal_terminated;
	api->terminal_input = api_terminal_input;
	api->terminal_input_chars = api_terminal_input_chars;
	api->terminal_refresh = api_terminal_refresh;
	api->primary_input = api_primary_input;
	api->primary_input_chars = api_primary_input_chars;
	api->primary_refresh = api_primary_refresh;

	PLUGIN_LIST_FOREACH_SAFE(&g_plugin_list, i, next) {
		fprintf(stderr, "initialize %s...\n", i->plugin.name);
		if( (*i->plugin.init)(&i->plugin, api) != 0) {
			fprintf(stderr, "\tinit for %s failed\n", i->plugin.name);
			plugin_list_del(&g_plugin_list, i);
		}
	}
}

static void free_plugins() {
	struct aug_plugin_item *i;

	AUG_LOCK(&g_free_plugin_lock);
	PLUGIN_LIST_FOREACH_REV(&g_plugin_list, i) {
		fprintf(stderr, "free %s...\n", i->plugin.name);
		(*i->plugin.free)();
	}
	AUG_UNLOCK(&g_free_plugin_lock);

	plugin_list_free(&g_plugin_list);
}

/* ============== MAIN ============================== */

static void main_to_lock_for_io(void *user) {
	(void)(user);

	lock_all();
}

static void main_to_unlock_after_io(void *user) {
	(void)(user);

	unlock_all();
}

static void to_refresh_after_io(void *user) {
	(void)(user);

	panel_stack_update();
	screen_doupdate();
}

int aug_main(int argc, char *argv[]) {
	struct termios child_termios;
	struct aug_api api;
	int rows, cols;

	switch(init_conf(argc, argv)) { /* 1 */
	case 0:
		/* everything is good. keep going */
		break;
	case 1:
		/* -h -> print help. this isnt an error */
		return 0;
	default:
		/* error. return 1 */
		return 1;
	}

	fprintf(stderr, "configuration:\n");
	conf_fprint(&g_conf, stderr);
	
	if(tcgetattr(STDIN_FILENO, &child_termios) != 0) {
		err_exit(errno, "tcgetattr failed");
	}

	setlocale(LC_ALL,"");
	
	/* set the PARENT terminal profile to --ncterm if supplied.
	 * have to set this before calling screen_init to affect ncurses */
	if(g_conf.ncterm != NULL)
		if(setenv("TERM", g_conf.ncterm, 1) != 0)
			err_exit(errno, "error setting environment variable: %s", g_conf.ncterm);

	/* block sigs right off the bat because we want to defer 
	 * processing of it until curses and vterm are set up. 
	 * the signal threads will get setup to process signals
	 * just before we enter the main I/O loop.
	 */
	block_sigs();	

	fprintf(stderr, "initialize primary terminal\n");
	/* screen will resize term to the right size,
	 * so just initialize to 1x1. */
	term_init(&g_term, 1, 1); /* 2 */
	fprintf(stderr, "initialize screen\n");
	if(screen_init(&g_term) != 0) /* 3 */
		err_exit(0, "screen_init failure");
	err_exit_cleanup_fn(err_exit_cleanup);
	if(g_conf.nocolor == false)
		if(screen_color_start() != 0) {
			err_warn(0, "failed to start color: %s", screen_err_msg(errno));
			goto screen_cleanup;
		}

	fprintf(stderr, "initialize child process\n");
	child_init(
		&g_child, 
		&g_term, 
		(char *const *) g_conf.cmd_argv,
		child_setup,
		to_refresh_after_io,
		main_to_lock_for_io,
		main_to_unlock_after_io,
		&child_termios,
		NULL
	);
	fprintf(stderr, "started primary child at pid %d\n", g_child.pid);

	AUG_LOCK_INIT(&g_screen); /* 4 */
	AUG_LOCK_INIT(&g_free_plugin_lock);
	/* init keymap structure */
	keymap_init(&g_keymap); /* 5 */

	objset_init(&g_edgewin_set); /* 6 */
	region_map_init(); 
	AUG_LOCK_INIT(&g_region_map);
	g_tchild_table.tree = avl_new( (AvlCompare) void_compare );
	AUG_LOCK_INIT(&g_tchild_table);
		
	/* this is first point where api functions can be called
	 * and locks will be utilized */
	AUG_LOCK(&g_free_plugin_lock);
	init_plugins(&api); /* 7 */
	AUG_UNLOCK(&g_free_plugin_lock);
	child_lock(&g_child); 
	g_plugins_initialized = true;
	child_unlock(&g_child);
	/* now that plugins have been initialized, callbacks can
	 * start happening. */
	
	/* make initial callback to plugins so they
	 * know what the screen dims are right now, as the
	 * only other way they will get this callback is from
	 * a sigwinch, which may never happen. */
	lock_all();
	screen_dims(&rows, &cols);
	aug_screen_dims_change(rows, cols);
	screen_term_win_dims(&rows, &cols);
	aug_primary_term_dims_change(rows, cols);
	unlock_all();

	fprintf(stderr, "lock screen\n");
	/* this calls main_to_lock_for_io. resources will be 
	 * unlocked in child_io_loop by calling main_to_unlock_for_io.
	 * this will block signals a second time, but that shouldnt
	 * be a problem. */
	child_lock(&g_child);

	/* get ncurses SIGWINCH handler */
	if(sigaction(SIGWINCH, NULL, &g_prev_winch_act) != 0)
		err_exit(errno, "sigaction failed");
	start_sig_threads();

	screen_redraw_term_win();
	fprintf(stderr, "start main event loop\n");
	/* main event loop */	
	child_io_loop(
		&g_child,
		STDIN_FILENO,
		process_keys
	);
	/* everything should be unlocked at this point */
	fprintf(stderr, "end main event loop, exiting...\n");

	/* cleanup */
	/* no longer want to explicitly reap children */
	stop_chld_thread();
	stop_winch_thread();
	free_plugins(); /* 7 */

	/* plugins should have handled cleanup
	 * of children processes. (maybe put a warning here?) */
	avl_free(g_tchild_table.tree);
	AUG_LOCK_FREE(&g_tchild_table);

	region_map_free(); /* 6 */
	AUG_LOCK_FREE(&g_region_map);
	objset_clear(&g_edgewin_set);

	keymap_free(&g_keymap); /* 5 */
	AUG_LOCK_FREE(&g_free_plugin_lock);
	AUG_LOCK_FREE(&g_screen); /* 4 */
	child_free(&g_child);
screen_cleanup:
	screen_free(); /* 3 */
	term_free(&g_term); /* 2 */
	
	if(g_ini != NULL) 
		ciniparser_freedict(g_ini); /* 1 */
	conf_free(&g_conf);
	
	return 0;
}

