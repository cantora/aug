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
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include <fcntl.h>

#if defined(__FreeBSD__)
#	include <libutil.h>
#	include <termios.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#	include <termios.h>
#	include <util.h>
#else
#	include <pty.h>
#endif

#include "vterm.h"

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

static void lock_all();
static void unlock_all();
static void lock_screen();
static void unlock_screen();

static struct aug_conf g_conf; /* structure of configuration variables */
static struct aug_plugin_list g_plugin_list;
static bool g_plugins_initialized = false;
static struct aug_term g_term;
static dictionary *g_ini;	
static struct aug_keymap g_keymap;
static struct {
	AUG_LOCK_MEMBERS;
} g_screen;

#define BUF_SIZE 2048*4
static char g_buf[BUF_SIZE]; /* IO buffer */
/* static globals */
static struct sigaction g_winch_act, g_prev_winch_act; /* structs for handling window size change */

/* ================= API FUNCTIONS ==================================== */

static int api_log(struct aug_plugin *plugin, const char *format, ...) {
	va_list args;
	int result;

	fprintf(stderr, "%s: ", plugin->name);
	va_start(args, format);
	result = vfprintf(stderr, format, args);
	va_end(args);

	return result;
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

	if(callbacks != NULL)
		plugin->callbacks = callbacks;
}

static void api_stack_size(struct aug_plugin *plugin, int *size) {
	struct aug_plugin_item *itr;
	int i;
	(void)(plugin);

	AUG_LOCK(&g_plugin_list);
	i = 0;
	PLUGIN_LIST_FOREACH(&g_plugin_list, itr) {
		i++;
	}
	AUG_UNLOCK(&g_plugin_list);

	*size = i;
}

static int api_stack_pos(struct aug_plugin *plugin, const char *name, int *pos) {
	struct aug_plugin_item *itr;
	int i, status;
	(void)(plugin);

	status = -1;
	AUG_LOCK(&g_plugin_list);
	i = 0;
	PLUGIN_LIST_FOREACH(&g_plugin_list, itr) {
		if(strncmp(itr->plugin.name, name, AUG_MAX_PLUGIN_NAME_LEN) == 0) {
			*pos = i;
			status = 0;
			goto unlock;
		}
		i++;
	}

unlock:
	AUG_UNLOCK(&g_plugin_list);	

	return status;
}

static void api_term_win_dims(struct aug_plugin *plugin, int *rows, int *cols) {
	(void)(plugin);

	AUG_LOCK(&g_term);
	term_dims(&g_term, rows, cols);
	AUG_UNLOCK(&g_term);
}

static int api_key_bind(const struct aug_plugin *plugin, int ch, 
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

static int api_key_unbind(const struct aug_plugin *plugin, int ch) {
	int result;
	(void)(plugin);
	
	AUG_LOCK(&g_keymap);
	result = keymap_unbind(&g_keymap, ch);
	AUG_UNLOCK(&g_keymap);

	return result;
}

static void api_screen_panel_alloc(struct aug_plugin *plugin, int nlines, int ncols, 
								int begin_y, int begin_x, PANEL **panel) {
	lock_screen();
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

	lock_screen();
	panel_stack_update();
	unlock_screen();
}

static void api_screen_doupdate(struct aug_plugin *plugin) {
	(void)(plugin);

	lock_screen();
	screen_doupdate();
	unlock_screen();
}

/* =================== end API functions ==================== */

/* ================= term callbacks for API =========================== */

int aug_cell_update(int *row, int *col, wchar_t *wch, attr_t *attr, int *color_pair) {
	struct aug_plugin_item *i;
	aug_action action;

	if(g_plugins_initialized != true)
		return 0;

	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if(i->plugin.callbacks == NULL || i->plugin.callbacks->cell_update == NULL)
			continue;

		unlock_all();		
		(*(i->plugin.callbacks->cell_update))(row, col, wch, attr, color_pair,
												 &action, i->plugin.callbacks->user);
		lock_all();

		if(action == AUG_ACT_CANCEL) /* plugin wants to filter this cell update */
			return -1;
	}
	
	return 0;
}

int aug_cursor_move(int old_row, int old_col, int *new_row, int *new_col) {
	struct aug_plugin_item *i;
	aug_action action;

	if(g_plugins_initialized != true)
		return 0;
		
	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if(i->plugin.callbacks == NULL || i->plugin.callbacks->cursor_move == NULL)
			continue;
	
		unlock_all();	
		(*(i->plugin.callbacks->cursor_move))(old_row, old_col, new_row, new_col,
												 &action, i->plugin.callbacks->user);
		lock_all();

		if(action == AUG_ACT_CANCEL) /* plugin wants to filter this cell update */
			return -1;
	}

	return 0;
}

/* MUST LOCK for API access to the following modules:
 * screen_*
 * panel_stack_*
 */
static void lock_screen() {
	AUG_LOCK(&g_term);
	AUG_LOCK(&g_screen);
}

static void unlock_screen() {
	AUG_UNLOCK(&g_screen);
	AUG_UNLOCK(&g_term);
}

static void lock_all() {
	AUG_LOCK(&g_keymap);
	AUG_LOCK(&g_plugin_list);
	lock_screen();
}

static void unlock_all() {
	unlock_screen();
	AUG_UNLOCK(&g_plugin_list);
	AUG_UNLOCK(&g_keymap);
}

static void change_winch(int how) {
	sigset_t set;

	if(sigemptyset(&set) != 0) 
		err_exit(errno, "sigemptyset failed"); 

	if(sigaddset(&set, SIGWINCH) != 0) 
		err_exit(errno, "sigaddset failed");

	if(sigprocmask(how, &set, NULL) != 0)
		err_exit(errno, "sigprocmask failed");
}

static inline void block_winch() {
	change_winch(SIG_BLOCK);
}

static inline void unblock_winch() {
	change_winch(SIG_UNBLOCK);
}

/* handler for SIGWINCH. */
static void handler_winch(int signo) {
	int rows, cols;
	struct aug_plugin_item *i;

	fprintf(stderr, "handler_winch: enter\n");
	lock_screen(); /* locks screen AND term */
	fprintf(stderr, "handler_winch: locked screen\n");

	vterm_screen_flush_damage(vterm_obtain_screen(g_term.vt) );

	/* in case curses installed a winch handler */
	if(g_prev_winch_act.sa_handler != NULL) {
		fprintf(stderr, "call previous winch handler\n");
		(*g_prev_winch_act.sa_handler)(signo);
	}

	/* tell the screen manager to resize to whatever the new
	 * size is (curses knows already). the screen manager
	 * resizes all its windows, then tells the terminal window
	 * manager and plugins about the resize */
	screen_resize();
	unlock_screen();

	screen_dims(&rows, &cols);	
	/* plugin callbacks */
	if(g_plugins_initialized == true) {
		PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
			if(i->plugin.callbacks == NULL || i->plugin.callbacks->screen_dims_change == NULL)
				continue;
			
			(*(i->plugin.callbacks->screen_dims_change))(rows, cols, i->plugin.callbacks->user);
		}
	}

	fprintf(stderr, "handler_winch: unlocked screen\nhandler_winch: exit\n");
}

static void push_key(VTerm *vt, int ch) {
	struct aug_plugin_item *i;
	aug_action action;
	
	PLUGIN_LIST_FOREACH(&g_plugin_list, i) {
		if(i->plugin.callbacks == NULL || i->plugin.callbacks->input_char == NULL)
			continue;
		
		unlock_all();
		(*(i->plugin.callbacks->input_char))(&ch, &action, i->plugin.callbacks->user);
		lock_all();

		if(action == AUG_ACT_CANCEL) /* plugin wants to filter this character */
			return;
	}

	vterm_input_push_char(vt, VTERM_MOD_NONE, (uint32_t) ch);
}

static void process_keys(VTerm *vt) {
	int ch;
	static bool command_key = false;
	aug_on_key_fn command_fn;
	void *user;

	/* we need at least two spots in the buffer because in 'pass through' 
	 * command prefix mode we will send both the command key and the following
	 * key at the same time after finding a non-command */
	while(vterm_output_get_buffer_remaining(vt) > 1 && screen_getch(&ch) == 0 ) {
		if(command_key == true) { /* treat *ch* as a command extension */
			fprintf(stderr, "check for command extension 0x%02x\n", ch);
			keymap_binding(&g_keymap, ch, &command_fn, &user);
			if(command_fn == NULL) { /* this is not a bound key */
				push_key(vt, g_conf.cmd_key);
				push_key(vt, ch);
			}
			else { /* invoke the command */
				unlock_all(); /* note: WINCH is should still be blocked */
				(*command_fn)(ch, user);
				lock_all();
			}
			command_key = false;
		}
		else if(ch == g_conf.cmd_key) {
			command_key = true;	
		}
		else {
			push_key(vt, ch);
		}
	}
}

static void process_vterm_output(VTerm *vt, int master) {
	size_t buflen;

	while( (buflen = vterm_output_get_buffer_current(vt) ) > 0) {
		buflen = (buflen < BUF_SIZE)? buflen : BUF_SIZE;
		buflen = vterm_output_bufferread(vt, g_buf, buflen);
		write_n_or_exit(master, g_buf, buflen);
	}
}

static int process_master_output(VTerm *vt, int master) {
	ssize_t n_read, total_read;

	total_read = 0;
	do {
		n_read = read(master, g_buf + total_read, 512);
	} while(n_read > 0 && ( (total_read += n_read) + 512 <= BUF_SIZE) );
	
	if(n_read == 0 || (n_read < 0 && errno == EIO) ) { 
		return -1; /* the master pty is closed, return -1 to signify */
	}
	else if(n_read < 0 && errno != EAGAIN) {
		err_exit(errno, "error reading from pty master (n_read = %d)", n_read);
	}

	vterm_push_bytes(vt, g_buf, total_read);
	
	return 0;
}

static void loop(struct aug_term *term) {
	fd_set in_fds;
	int status, force_refresh, just_refreshed;

	struct aug_timer inter_io_timer, refresh_expire;
	struct timeval tv_select;
	struct timeval *tv_select_p;

	timer_init(&inter_io_timer);
	timer_init(&refresh_expire);
	/* dont initially need to worry about inter_io_timer's need to timeout */
	just_refreshed = 1;

	while(1) {
		FD_ZERO(&in_fds);
		FD_SET(STDIN_FILENO, &in_fds);
		FD_SET(term->master, &in_fds);

		/* if we just refreshed the screen there
		 * is no need to timeout the select wait. 
		 * however if we havent refreshed on the last
		 * iteration we need to make sure that inter_io_timer
		 * is given a chance to timeout and cause a refresh.
		 */
		tv_select.tv_sec = 0;
		tv_select.tv_usec = 1000;
		tv_select_p = (just_refreshed == 0)? &tv_select : NULL;

		/* most of this process's time is spent waiting for
		 * select's timeout, so this is where we want to handle all
		 * SIGWINCH signals and MOST (all?) of the asynchronous
		 * API calls.
		 */
#		define LOOP_UNLOCK() \
			do { unlock_all(); unblock_winch(); } while(0)

#		define LOOP_LOCK() \
			do { block_winch(); lock_all(); } while(0)
			
		LOOP_UNLOCK(); /* winch handler needs to lock the screen. */
		/* if a plugin thread is caused to handle
		 * a WINCH signal there could be trouble because the plugin may
		 * have locked the screen via an API call, then gets interrupted by a WINCH
		 * signal which will attempt to lock the screen again. of course
		 * this isnt a problem for this main thread here because it only
		 * handles WINCH signals while in the following select call. so...
		 * this means we have to block WINCH and (maybe other signals?) during
		 * periods where the main thread may call a plugin function, such that
		 * when the plugin creates a thread it will inherit a signal mask
		 * with WINCH blocked. the API will expect plugins not to unblock that
		 * signal in their threads.
		 */
		if(select(term->master + 1, &in_fds, NULL, NULL, tv_select_p) == -1) {
			if(errno == EINTR) {
				LOOP_LOCK();				
				continue;
			}
			else
				err_exit(errno, "select");
		}		
		LOOP_LOCK(); /* need exclusive access to the screen for all IO */

		if(FD_ISSET(term->master, &in_fds) ) {
			if(process_master_output(term->vt, term->master) != 0) {
				LOOP_UNLOCK();
				return;
			}

			process_vterm_output(term->vt, term->master);
			timer_init(&inter_io_timer);
		}

#		undef LOOP_LOCK
#		undef LOOP_UNLOCK

		/* this is here to make sure really long bursts dont 
		 * look like frozen I/O. the user should see characters
		 * whizzing by if there is that much output.
		 */
		if( (status = timer_thresh(&refresh_expire, 0, 50000)) ) { 
			force_refresh = 1; /* must refresh at least 20 times per second */
		}
		else if(status < 0)
			err_exit(errno, "timer error");
		
		/* if master pty is 'bursting' with I/O at a quick rate
		 * we want to let the burst finish (up to a point: see refresh_expire)
		 * and then refresh the screen, otherwise we waste a bunch of time
		 * refreshing the screen with stuff that just gets scrolled off
		 */
		if(force_refresh != 0 || (status = timer_thresh(&inter_io_timer, 0, 700) ) == 1 ) {
			if(term->io_callbacks.refresh != NULL)
				(*term->io_callbacks.refresh)(term->user); /* call the term refresh callback */

			//touchwin(stdscr);
			panel_stack_update();
			//screen_refresh();
			screen_doupdate();
					
			timer_init(&inter_io_timer);
			timer_init(&refresh_expire);
			force_refresh = 0;
			just_refreshed = 1;
		}
		else if(status < 0)
			err_exit(errno, "timer error");
		else
			just_refreshed = 0; /* didnt refresh the screen on this iteration */

		if(FD_ISSET(STDIN_FILENO, &in_fds) ) {
			process_keys(term->vt);
			process_vterm_output(term->vt, term->master);
			force_refresh = 1;
		} /* if stdin set */
	} /* while(1) */
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

	conf_init(&g_conf);
	if(opt_parse(argc, argv, &g_conf) != 0) {
		switch(errno) {
		case OPT_ERR_HELP:
			opt_print_help(argc, (const char *const *) argv);
			break;
			
		case OPT_ERR_USAGE:
			opt_print_usage(argc, (const char *const *) argv);
			break;
		
		default:
			fprintf(stdout, "%s\n", opt_err_msg);
			opt_print_usage(argc, (const char *const *) argv);
			fputc('\n', stdout);
		}	
		
		return 1;
	}

	if(access(g_conf.conf_file, R_OK) == 0) {
		g_ini = ciniparser_load(g_conf.conf_file);
		if(g_ini != NULL) {
			have_config = true;
			conf_merge_ini(&g_conf, g_ini);
		}
	}
	else
		config_err = errno;

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
		fprintf(stdout, "%s\n", errmsg);
		return 1;
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

	fprintf(stderr, "load plugins...\n");
	if(g_ini == NULL)
		return;

	if( (nsec = ciniparser_getnsec(g_ini) ) < 1)
		return;

	secname = NULL;
	
	for(i = 1; i <= nsec; i++) {
		secname = ciniparser_getsecname(g_ini, i);
		assert(secname != NULL);

		/*fprintf(stderr, "section %d: %s\n", i, secname);*/
		if( strcmp(secname, CONF_CONFIG_SECTION_CORE) == 0 )
			continue;

		seclen = strlen(secname);
		if(seclen > AUG_MAX_PLUGIN_NAME_LEN) {  /* seems a bit excessive... */
			fprintf(stderr, "plugin section name exceeds maximum plugin name length of %d\n", AUG_MAX_PLUGIN_NAME_LEN);
			continue;
		}

		err = "could not find plugin in search path";
		/* this is a plugin section so we want to 
		 * go looking for it in the plugin_path */
		fprintf(stderr, "search for plugin: %s\n", secname);
		TOK_ITR_FOREACH(path, (1024-seclen-3-1), g_conf.plugin_path, ':', &tok_itr) {
			size_t len;

			len = strlen(path);
			fprintf(stderr, "\tsearching in %s\n", path);
			if(path[len-1] != '/') {
				path[len++] = '/';
				path[len] = '\0';
			}
				
			strcat(path, secname);
			strcat(path, ".so");

			if(access(path, R_OK) == 0) {
				/* load the plugin */
				err = NULL;
				plugin_list_push(&g_plugin_list, path, secname, seclen, &err);
				if(err == NULL)
					fprintf(stderr, "\tloaded %s.so\n", secname);

				break;
			}
		} /* FOREACH */
		
		if(err != NULL)	
			fprintf(stderr, "\terror loading plugin %s: %s\n", secname, err);

	} /* for each section */

}

static void child_init(const char *env_term) {
	const char *child_term;

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

	unblock_winch();	
	execvp(g_conf.cmd_argv[0], (char *const *) g_conf.cmd_argv);
}

static void init_plugins(struct aug_api *api) {
	struct aug_plugin_item *i, *next;

	plugin_list_init(&g_plugin_list);
	load_plugins();
	api->log = api_log;
	api->callbacks = api_callbacks;
	api->conf_val = api_conf_val;
	api->stack_size = api_stack_size;
	api->stack_pos = api_stack_pos;
	api->term_win_dims = api_term_win_dims;
	api->key_bind = api_key_bind;
	api->key_unbind = api_key_unbind;

	api->screen_panel_alloc = api_screen_panel_alloc;
	api->screen_panel_dealloc = api_screen_panel_dealloc;
	api->screen_panel_size = api_screen_panel_size;
	api->screen_panel_update = api_screen_panel_update;
	api->screen_doupdate = api_screen_doupdate;

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

	PLUGIN_LIST_FOREACH_REV(&g_plugin_list, i) {
		fprintf(stderr, "free %s...\n", i->plugin.name);
		(*i->plugin.free)();
	}

	plugin_list_free(&g_plugin_list);
}

/* ============== MAIN ==============================*/

int aug_main(int argc, char *argv[]) {
	pid_t child;
	struct winsize size;
	const char *env_term;
	struct termios child_termios;
	int master;
	struct aug_api api;

	if(init_conf(argc, argv) != 0) /* 1 */
		return 1;

	if(tcgetattr(STDIN_FILENO, &child_termios) != 0) {
		err_exit(errno, "tcgetattr failed");
	}

	setlocale(LC_ALL,"");
	
	env_term = getenv("TERM"); 
	/* set the PARENT terminal profile to --ncterm if supplied.
	 * have to set this before calling screen_init to affect ncurses */
	if(g_conf.ncterm != NULL)
		if(setenv("TERM", g_conf.ncterm, 1) != 0)
			err_exit(errno, "error setting environment variable: %s", g_conf.ncterm);


	/* block winch right off the bat because we want to defer 
	 * processing of it until curses and vterm are set up. winch
	 * will get unblocked in the loop function.
	 */
	block_winch();
	g_winch_act.sa_handler = handler_winch;
	sigemptyset(&g_winch_act.sa_mask);
	g_winch_act.sa_flags = 0;

	/* screen will resize term to the right size,
	 * so just initialize to 1x1. */
	term_init(&g_term, 1, 1); /* 2 */
	if(screen_init(&g_term) != 0) /* 3 */
		err_exit(0, "screen_init failure");
	err_exit_cleanup_fn(err_exit_cleanup);
	
	term_dims(&g_term, (int *) &size.ws_row, (int *) &size.ws_col);

	if(g_conf.nocolor == false)
		if(screen_color_start() != 0) {
			printf("failed to start color\n");
			goto screen_cleanup;
		}

	child = forkpty(&master, NULL, &child_termios, &size);
	if(child == 0) {
		child_init(env_term);
		err_exit(errno, "cannot exec %s", g_conf.cmd_argv[0]);
	}

	if(set_nonblocking(master) != 0)
		err_exit(errno, "failed to set master fd to non-blocking");
	if(term_set_master(&g_term, master) != 0)
		err_exit(errno, "failed to set master pty in term structure");
	if(sigaction(SIGWINCH, &g_winch_act, &g_prev_winch_act) != 0)
		err_exit(errno, "sigaction failed");

	AUG_LOCK_INIT(&g_screen); /* 4 */
	/* init keymap structure */
	keymap_init(&g_keymap); /* 5 */
	/* this is first point where api functions will be called
	 * and locks will be utilized */
	init_plugins(&api); /* 6 */
	g_plugins_initialized = true;

	fprintf(stderr, "configuration:\n");
	conf_fprint(&g_conf, stderr);
	fprintf(stderr, "lock screen\n");
	lock_all(); /* will be unlocked in *loop* */
	fprintf(stderr, "start main event loop\n");
	/* main event loop */	
	loop(&g_term);
	/* everything should be unlocked at this point */
	fprintf(stderr, "end main event loop, exiting...\n");

	/* cleanup */
	block_winch(); 
	free_plugins(); /* 6 */
	unblock_winch();

	keymap_free(&g_keymap); /* 5 */
	AUG_LOCK_FREE(&g_screen); /* 4 */

screen_cleanup:
	screen_free(); /* 3 */
	term_free(&g_term); /* 2 */
	
	if(g_ini != NULL) 
		ciniparser_freedict(g_ini); /* 1 */

	
	return 0;
}

