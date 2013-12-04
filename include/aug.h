/* 
 * The MIT License (MIT)
 * Copyright (c) 2013 anthony cantor
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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

typedef enum {
	/* the default action in the context of the callback */
	AUG_ACT_OK = 0,
	/* cancel whatever would normally take place after this callback.
	 * for example, filter an input character */
	AUG_ACT_CANCEL
} aug_action;

/* callbacks which a plugin can register
 * interest in. unless otherwise specified
 * the output parameters of all the callbacks
 * will be initialized to current/default 
 * values, so a simple { return; } function
 * pointer will be the same as a NULL 
 * callback. 
 * the plugin should not make any api calls from within
 * a callback, as this will result in thread lock. the only
 * exceptions are screen_doupdate, screen_panel_update, log,
 * conf_val, terminal_pid and terminal_terminated.
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

struct aug_inject {
	const uint32_t *chars;
	size_t len;
};

struct aug_plugin_cb {
	/* called when a character of input
	 * is received from stdin. the plugin can
	 * update the ch variable and set action
	 * to AUG_ACT_OK in order to change the
	 * data being sent to the terminal. or 
	 * the plugin can set action to 
	 * AUG_ACT_CANCEL to cause the input
	 * to be filtered from the terminal. */
	void (*input_char)(uint32_t *ch, aug_action *action, struct aug_inject *inject, void *user);

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

	/* called when the terminal lines are about to
	 * be scrolled. if @direction is positive,
	 * the screen is being scrolled up by that amount; 
	 * that is, the lines are moving up the screen by
	 * @direction lines. setting @*action to AUG_ACT_CANCEL
	 * will prevent the use of ncurses scrolling functions
	 * and the parts of the screen which would be scrolled
	 * will instead be re-rendered (thus, the cell_update
	 * callback will be invoked for each cell in the scrolled
	 * area of the screen). */
	void (*pre_scroll)(
		int rows, int cols, int direction,
		aug_action *action, 
		void *user
	);

	/* called after terminal lines have been scrolled.
	 * @direction follows the same behaviour as in the
	 * pre_scroll callback. if @action is set to AUG_ACT_CANCEL
	 * the post_scroll callback will not be invoked for
	 * plugins after this one. */
	void (*post_scroll)(
		int rows, int cols, int direction,
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
	
	/* called when the primary terminal dimensions change. */
	void (*primary_term_dims_change)(int rows, int cols, void *user);

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
	 * if non-zero is returned the plugin will not
	 * be loaded into the plugin stack (the free
	 * function will not be called).
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
	 * there are two types of screen real estate a plugin can 
	 * request the aug core to allocate: panels and windows. 
	 * panels correspond to the ncurses panels library
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

	/* neither of the following three api calls engage any locks
	 * so lock_screen and unlock_screen should be used
	 * to ensure these calls are made safely. in the case of
	 * api callbacks, these can be called without using the
	 * lock_screen/unlock_screen api calls. */	

	/* call this function instead of
	 * calling update_panels() from the panels library. */
	void (*screen_panel_update)(struct aug_plugin *plugin);

	/* call this instead of calling doupdate() */
	void (*screen_doupdate)(struct aug_plugin *plugin);

	/* mark for redrawing the areas of the screen described by the rectangle
	 * given by @col_start, @col_end, @row_start and @row_end. */
	void (*primary_term_damage)(const struct aug_plugin *plugin, 
			size_t col_start, size_t col_end, size_t row_start, size_t row_end);



	/* fork a child process according to @argv and allocate 
	 * a terminal attached to the child. @twin must point to
	 * the window associated with the terminal. @terminal will
	 * be assigned to the result and must be passed to other
	 * terminal_* api calls. */
	void (*terminal_new)(struct aug_plugin *plugin, struct aug_terminal_win *twin,
							char *const *argv, void **terminal);

	/* delete a previously allocated terminal @terminal. this will not clear or
	 * destroy the window (@twin from above) or actually kill the child
	 * process, it will simply deallocate memory resources. if the child
	 * should be killed, use terminal_pid to get the PID of the process and
	 * send it a signal before calling this function.
	 * obviously, do not call any other api call with @terminal after
	 * deleting it (for example, one thread deletes the terminal and
	 * another is still writing into it). */
	void (*terminal_delete)(struct aug_plugin *plugin, void *terminal);

	/* return the process id of the child process spawned for @terminal */
	pid_t (*terminal_pid)(struct aug_plugin *plugin, const void *terminal);
	
	/* returns 0 if the process associated with the terminal has not
	 * terminated. otherwise non-zero is returned. */
	int (*terminal_terminated)(struct aug_plugin *plugin, const void *terminal);

	/* this is the I/O loop for @terminal, it watches the pty file descriptor
	 * and writes to/refreshes the ncurses window when appropriate. you will
	 * likely create a thread specifically for the purpose of calling this 
	 * function, as this will not return until the child process has exited. 
	 *
	 * NOTE: you must ensure this function has exited before calling 
	 * terminal_delete on @terminal. probably the cleanest way to cleanup
	 * @terminal is to kill the child process and join the thread that called
	 * this function (this function will return when the child process exits).
	 * after joining the thread you can free the memory resources by passing
	 * @terminal to terminal_delete.
	 */
	void (*terminal_run)(struct aug_plugin *plugin, void *terminal);

	/* input utf-32 (wide characters/wchar_t) to the terminal. characters
	 * passed into this function are the input/keystrokes into @terminal. 
	 * this function returns the number of characters that were written
	 * to the terminal (there is a limit to number of characters that
	 * can be written at any given time; call this function repeatedly
	 * in a loop until it has written everything). */
	size_t (*terminal_input)(struct aug_plugin *plugin, void *terminal, 
								const uint32_t *data, int n);

	/* same as above except the input is ASCII instead of UTF-32 */
	size_t (*terminal_input_chars)(struct aug_plugin *plugin, void *terminal, 
								const char *data, int n);

	/* refreshes (and flushes the damage) of the terminal to cause output
	 * to the screen. you probably want to run screen_panel_update and
	 * screen_doupdate after calling this function. */
	void (*terminal_refresh)(struct aug_plugin *plugin, void *terminal);

	/* similar to the terminal input api calls, these allow
	 * a plugin to write input keys/data to the primary terminal. 
	 * this will be very useful to automate input into the primary
	 * terminal. */
	size_t (*primary_input)(struct aug_plugin *plugin,
								const uint32_t *data, int n);
	size_t (*primary_input_chars)(struct aug_plugin *plugin,
									const char *data, int n);

	/* refreshes (and flushes the damage) of the primary terminal to 
	 * cause output to the screen. you probably want to run 
	 * screen_doupdate after calling this function. */
	void (*primary_refresh)(struct aug_plugin *plugin);
};

#endif /* AUG_AUG_H */
