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
	void (*input_char)(int *ch, aug_action *action, void *user);

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
typedef void (*aug_on_key_fn)(int chr, void *user);

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
	int (*key_bind)(const struct aug_plugin *plugin, int ch, aug_on_key_fn on_key, void *user );
	int (*key_unbind)(const struct aug_plugin *plugin, int ch);

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
	 * the screen_win_alloc_* call or it there is not enough space, the
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
		void (*callback)(WINDOW *win, void *user) 
	); 

	void (*screen_win_alloc_bot)(
		struct aug_plugin *plugin, 
		int nlines, 
		void (*callback)(WINDOW *win, void *user) 
	); 

	void (*screen_win_alloc_left)(
		struct aug_plugin *plugin, 
		int ncols, 
		void (*callback)(WINDOW *win, void *user) 
	); 

	void (*screen_win_alloc_right)(
		struct aug_plugin *plugin, 
		int ncols, 
		void (*callback)(WINDOW *win, void *user) 
	); 

	/* return = 0 if a window was deallocated.
	 * return non-zero if no window associated with callback
	 * was found.
	 */
	int (*screen_win_dealloc)(struct aug_plugin *plugin, \
				void (*callback)(WINDOW *win, void *user)); 

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
							char *const *argv, void **terminal, int *pipe_fd);
	pid_t (*terminal_pid)(struct aug_plugin *plugin, const void *terminal);
	void (*terminal_run)(struct aug_plugin *plugin, void *terminal);
	void (*terminal_delete)(struct aug_plugin *plugin, void *terminal);
};

#endif /* AUG_AUG_H */
