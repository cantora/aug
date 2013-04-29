/* 
 * Copyright 2013 anthony cantor
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
#ifndef AUG_AUG_H
#define AUG_AUG_H

#include <stdint.h>
#include <wchar.h>
#include "ncurses.h"
#include <panel.h>
#include <unistd.h>

#define AUG_API_VERSION_MAJOR 0
#define AUG_API_VERSION_MINOR 0

/* defined below */
struct aug_api;
struct aug_plugin;

typedef enum { AUG_ACT_OK = 0, AUG_ACT_CANCEL } aug_action;

/* callbacks which a plugin can register
 * interest in. unless otherwise specified
 * the output parameters of all the callbacks
 * will be initialized to current/default 
 * values, so a simple { return; } function
 * pointer will be the same as a NULL 
 * callback. 
 * the plugin should not make any api calls from within
 * a callback, as this will result in thread lock. the only
 * exceptions are screen_doupdate and screen_panel_update.
 * warning: also do not wait on any locks which another
 * thread may have locked when that other thread may be making
 * api calls; this will cause a circular dependency and most
 * likely the threads will become deadlocked. example:
 *    __________________________________________________
 *   | aug thread             |   plugin thread1        |
 *   |------------------------|-------------------------|
 *   |                        |   *locks mtx1*          |
 *   |  *locks api*           |                         |
 *   |  invokes api callback  |                         |
 *   |(now in plugin callback)|                         |
 *   |*locks mtx1* (blocked)  |                         | <- (1)
 *   |                        | makes api call (blocked)| <- (2)
 *   |------------------------|-------------------------|
 *   |                    DEADLOCKED                    |
 *
 *   (1) the aug thread cant unlock the api until the callback completes
 *   (2) the callback will never complete until thread1 does its business 
 *       and unlocks mtx1, but thread1 is blocked on the api call because 
 *       the api is locked.
 */
struct aug_plugin_cb {
	/* called when a unit of input
	 * is received from stdin. the plugin can
	 * update the ch variable and set action
	 * to AUG_ACT_OK in order to change the
	 * data being sent to the terminal. or 
	 * the plugin can set action to 
	 * AUG_ACT_CANCEL to cause the input
	 * to be filtered from the terminal. */
	void (*input_char)(uint32_t *ch, aug_action *action, void *user);

	/* called when a cell in the terminal window is
	 * about to be updated. the plugin can
	 * alter (or not) any of the output 
	 * variables and then
	 * set action to AUG_ACTION_OK to modify
	 * the cell getting updated. or it can
	 * set action to AUG_ACTION_CANCEL and 
	 * and prevent the cell from being 
	 * updated. */
	void (*cell_update)(
		int rows, int cols,
		int *row, int *col, 
		wchar_t *wch, attr_t *attr,
		int *color_pair, 
		aug_action *action, 
		void *user
	);

	/* called when the cursor is about to be
	 * moved to some location in the terminal window.
	 * the plugin can change (or not) the 
	 * new_row, new_col output parameters
	 * and set action to AUG_ACT_OK to modify
	 * where the cursor is moved to. */
	void (*cursor_move)(
		int rows, int cols, 
		int old_row, int old_col, 
		int *new_row, int *new_col,
		aug_action *action, void *user
	);

	/* called when the screen dimensions change.
	 * note that the dimensions are for the entire screen, not
	 * the size of the main window. */
	void (*screen_dims_change)(int rows, int cols, void *user);

	void *user;
};

/* this structure represents the 
 * application's view of the plugin.
 * PLUGINS SHOULD NOT MODIFY THIS 
 * DIRECTLY, but rather should only
 * pass it to api functions.
 */
struct aug_plugin {

#define AUG_MAX_PLUGIN_NAME_LEN 255
	/* name symbol. this name
	 * will be used for option parsing
	 * and configuration file parsing
	 * to automatically provide this 
	 * plugin with user specified
	 * configuration values. */
	const char *const name;
	
	/* init and free symbols.
	 * return non-zero to signal error and result in 
	 * plugin getting unloaded from the plugin stack.
	 */
#define AUG_API_INIT_ARG_PROTO struct aug_plugin *plugin, const struct aug_api *api
	int (*const init)( AUG_API_INIT_ARG_PROTO );
#define AUG_API_FREE_ARG_PROTO     /* empty */
	void (*const free)( AUG_API_FREE_ARG_PROTO );

	/* callback subscriptions for this
	 * plugin. */
	const struct aug_plugin_cb *callbacks;

	/* handle to the loaded library */
	void *so_handle;
};

/* function pointer type for callbacks on key extensions */
typedef void (*aug_on_key_fn)(uint32_t chr, void *user);

struct aug_terminal_win {
	WINDOW *win;
};

/* passed to the plugin to provide 
 * an interface to the core application.
 * these functions are thread safe.
 */
struct aug_api {

	/* log a message to the debug log file. 
	 * returns the number of characters printed or
	 * a negative value on error.
	 */
	int (*log)(struct aug_plugin *plugin, const char *format, ...);
	
	/* convert a keyname such as ^R or ^C to its utf-32 value.
	 * returns 0 if a value was found for the string and returns
	 * non-zero if the value was not found.
	 */
	int (*keyname_to_key)(const struct aug_plugin *plugin, 
					const char *keyname, uint32_t *ch);

	/* unload the calling plugin. do not call this
	 * during aug_plugin_init or aug_plugin_free.
	 * this function can be called by a plugin created
	 * thread in order to unload itself from aug. this
	 * api call will first call the plugin's free function
	 * from a new spawned thread (in this way the plugin
	 * can fully cleanup all its resources, including its
	 * joinable threads, even the one that made the call
	 * to unload). after this it will delete the plugin
	 * from its list of active plugins. since this api call
	 * does everything from a detached thread, this function
	 * will return immediately.
	 */
	void (*unload)(struct aug_plugin *plugin);

	/* call this function to query for a
	 * user specified configuration value
	 * parsed from the command line or the
	 * configuration file. name is the name
	 * of the plugin, key is name of the 
	 * configuration variable, and val is 
	 * a pointer to the location of the
	 * variable value. if the return value
	 * is zero the key was successfully
	 * found. note that you can pass the name of
	 * a different plugin to find out configuration 
	 * info of other plugins or to find out if a
	 * specific plugin is loaded or not. 
	 */
#define AUG_MAX_PLUGIN_KEY_LEN 255
	int (*conf_val)(struct aug_plugin *plugin, const char *name, const char *key, const char **val);

	/* get or set the callbacks for this plugin. if *callbacks* is NULL,
	 * the current set of callbacks will not be changed.
	 * if *prev* is NULL, the prev set of callbacks will not be returned.
	 */
	void (*callbacks)(struct aug_plugin *plugin, const struct aug_plugin_cb *callbacks, const struct aug_plugin_cb **prev);

	/* the user can configure a global command key
	 * to use as a prefix and plugins can bind to
	 * this key + some extension. for example, the 
	 * user might set ^A as the command key, and a 
	 * plugin might bind to ^A n by passing ch = 0x6e. 
	 * if the return value is non-zero, the key is
	 * reserved or already bound. 
	 * a call to key_bind should be matched by a call
	 * to key_unbind as soon as the plugin no longer 
	 * needs to use the binding or when aug_plugin_free
	 * function is called.
	 */
	int (*key_bind)(const struct aug_plugin *plugin, uint32_t ch, aug_on_key_fn on_key, void *user );
	int (*key_unbind)(const struct aug_plugin *plugin, uint32_t ch);

	/* ======== screen windows/panels ======================== 
	 * there are two types of screen real estate
	 * a plugin can request the aug core to allocate: panels and 
	 * windows. panels correspond to the ncurses panels library
	 * and will stack on top of the main terminal window in the 
	 * order they are allocated (like a popup window for some
	 * sort of transient interaction with the user). windows
	 * correspond to ncurses windows and will not overlap the 
	 * main terminal, but will only be allocated in ways such
	 * that they occupy a number of lines at the top, bottom, left
	 * or right of the screen (this is for peristent existence of 
	 * plugin output on the screen, like a status bar or something).
	 * concurrency: generally these windows will probably be accessed
	 * by a thread created by the plugin. all the api calls use locks
	 * to maintain thread safe access to the api resources. any window
	 * or panel allocated to the plugin can be accessed directly with
	 * ncurses functions because these windows and panels are "owned"
	 * by the plugin and are not accessed by any other threads. the
	 * only exceptions are when there are specific api calls provided
	 * which specify things that should or should not be done.
	 */

	/* any ncurses calls should be enclosed between these two calls
	 * in order to serialize access to the ncurses library
	 */
	void (*lock_screen)(const struct aug_plugin *plugin);
	void (*unlock_screen)(const struct aug_plugin *plugin);

	/* as described above, this allocates an ncurses window object
	 * with a height of *nlines* on the top, bottom, or with a width 
	 * of *ncols* on the left or right of the screen. the callback 
	 * will be invoked when the window is allocated for the plugin or
	 * the screen is resized (thus reallocating a window for the plugin).
	 * the screen will be locked during this callback, so
	 * the plugin must not make any API calls which lock the screen. the 
	 * callback will either provide a window with at least as much width/height as requested by
	 * the screen_win_alloc_* call or if there is not enough space, the
	 * WINDOW pointer will be NULL which means the plugin does not have
	 * access to its window until there is enough free space that the callback
	 * is invoked with a non-null window.
	 *
	 * call the function with suffix top, bot, left and right for a window 
	 * on the top, bottom, left and right respectively. 
	 */
	void (*screen_win_alloc_top)(
		struct aug_plugin *plugin, 
		int nlines, 
		void (*window_cb)(WINDOW *win, void *user),
		void (*free_cb)(WINDOW *win, void *user)
	); 

	void (*screen_win_alloc_bot)(
		struct aug_plugin *plugin, 
		int nlines, 
		void (*window_cb)(WINDOW *win, void *user),
		void (*free_cb)(WINDOW *win, void *user)
	); 

	void (*screen_win_alloc_left)(
		struct aug_plugin *plugin, 
		int ncols, 
		void (*window_cb)(WINDOW *win, void *user),
		void (*free_cb)(WINDOW *win, void *user)
	); 

	void (*screen_win_alloc_right)(
		struct aug_plugin *plugin, 
		int ncols, 
		void (*window_cb)(WINDOW *win, void *user),
		void (*free_cb)(WINDOW *win, void *user)
	); 

	/* return = 0 if a window was deallocated.
	 * return non-zero if no window associated with window_cb
	 * was found. the relevant *free_cb callback will be invoked.
	 */
	int (*screen_win_dealloc)(struct aug_plugin *plugin, \
				void (*window_cb)(WINDOW *win, void *user)); 

	/* allocate a panel on top of the main terminal window and
	 * on top of all previously allocated (by this plugin or others)
	 * panels.           
	 */
	void (*screen_panel_alloc)(struct aug_plugin *plugin, int nlines, int ncols, 
									int begin_y, int begin_x, PANEL **panel);
	void (*screen_panel_dealloc)(struct aug_plugin *plugin, PANEL *panel);

	/* modifies *size* to the number of VISIBLE panels. the api
	 * can not currently tell a plugin how many panels (visible and invisible)
	 * are allocated. 
	 */
	void (*screen_panel_size)(struct aug_plugin *plugin, int *size);
	

	/* neither of the following two api calls engage any locks
	 * so lock_screen and unlock_screen should be used
	 * to ensure these calls are made safely. in the case of
	 * api callbacks, these can be called without using the
	 * lock_screen/unlock_screen api calls. */	

	/* call this function instead of
	 * calling update_panels() from the panels library. */
	void (*screen_panel_update)(struct aug_plugin *plugin);

	/* call this instead of calling doupdate() */
	void (*screen_doupdate)(struct aug_plugin *plugin);

	void (*terminal_new)(struct aug_plugin *plugin, struct aug_terminal_win *twin,
							char *const *argv, void **terminal);

	/* obviously, do not call any other api call with *terminal after
	 * deleting it (for example, one thread deletes the terminal and
	 * another is still writing into it). */
	void (*terminal_delete)(struct aug_plugin *plugin, void *terminal);
	pid_t (*terminal_pid)(struct aug_plugin *plugin, const void *terminal);
	
	/* informs whether or not the process associated with the terminal
	 * has terminated or not. the status returned by this 
	 * function will not change during plugin_init, any
	 * api callbacks or during plugin_free even if the process has 
	 * since terminated, as SIGCHLD is blocked by aug
	 * during those times. */
	int (*terminal_terminated)(struct aug_plugin *plugin, const void *terminal);

	void (*terminal_run)(struct aug_plugin *plugin, void *terminal);
	/* input utf-32 (wide characters/wchar_t) to the terminal */
	int (*terminal_input)(struct aug_plugin *plugin, void *terminal, 
								const uint32_t *data, int n);
	/* input ASCII to the terminal */
	int (*terminal_input_chars)(struct aug_plugin *plugin, void *terminal, 
								const char *data, int n);

	/* similar to the terminal input api calls, these allow
	 * a plugin to write input keys/data to the primary 
	 * terminal. */
	int (*primary_input)(struct aug_plugin *plugin,
								const uint32_t *data, int n);
	int (*primary_input_chars)(struct aug_plugin *plugin,
									const char *data, int n);

};

#endif /* AUG_AUG_H */
