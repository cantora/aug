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

#define BUF_SIZE 2048*4

/* globals */
static struct {
	struct sigaction winch_act, prev_winch_act; /* structs for handing window size change */
	struct aug_term_t term;
	char buf[BUF_SIZE]; /* IO buffer */
	struct aug_conf conf;
} g; 

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

/* handler for SIGWINCH */
static void handler_winch(int signo) {

	fprintf(stderr, "handler_winch in process %d\n", getpid());
	vterm_screen_flush_damage(vterm_obtain_screen(g.term.vt) );

	/* in case curses installed a winch handler */
	if(g.prev_winch_act.sa_handler != NULL) {
		fprintf(stderr, "call previous winch handler\n");
		(*g.prev_winch_act.sa_handler)(signo);
	}

	/* tell the screen manager to resize to whatever the new
	 * size is (curses knows already). the screen manager
	 * resizes all its windows, then tells the terminal window
	 * manager and plugins about the resize */
	screen_resize();
}

static void process_keys(VTerm *vt, int master) {
	int ch;

	while(vterm_output_get_buffer_remaining(vt) > 0 && screen_getch(&ch) == 0 ) {
		vterm_input_push_char(vt, VTERM_MOD_NONE, (uint32_t) ch);
	}
	
}

static void process_vterm_output(VTerm *vt, int master) {
	size_t buflen;

	while( (buflen = vterm_output_get_buffer_current(vt) ) > 0) {
		buflen = (buflen < BUF_SIZE)? buflen : BUF_SIZE;
		buflen = vterm_output_bufferread(vt, g.buf, buflen);
		write_n_or_exit(master, g.buf, buflen);
	}
}

static int process_master_output(VTerm *vt, int master) {
	ssize_t n_read, total_read;

	total_read = 0;
	do {
		n_read = read(master, g.buf + total_read, 512);
	} while(n_read > 0 && ( (total_read += n_read) + 512 <= BUF_SIZE) );
	
	if(n_read == 0 || (n_read < 0 && errno == EIO) ) { 
		return -1; /* the master pty is closed, return -1 to signify */
	}
	else if(n_read < 0 && errno != EAGAIN) {
		err_exit(errno, "error reading from pty master (n_read = %d)", n_read);
	}

	vterm_push_bytes(vt, g.buf, total_read);
	
	return 0;
}

void loop(struct aug_term_t *term) {
	fd_set in_fds;
	int status, force_refresh, just_refreshed;

	struct timer inter_io_timer, refresh_expire;
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
		 * select's timeout, so we want to handle all
		 * SIGWINCH signals here
		 */
		unblock_winch(); 
		if(select(term->master + 1, &in_fds, NULL, NULL, tv_select_p) == -1) {
			if(errno == EINTR) {
				block_winch();
				continue;
			}
			else
				err_exit(errno, "select");
		}
		block_winch();

		if(FD_ISSET(term->master, &in_fds) ) {
			if(process_master_output(term->vt, term->master) != 0)
				return;

			process_vterm_output(term->vt, term->master);
			timer_init(&inter_io_timer);
		}

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
			process_keys(term->vt, term->master);
			process_vterm_output(term->vt, term->master);
			force_refresh = 1;
		}
	}
}

static void err_exit_cleanup(int error) {
	(void)(error);

	screen_free();
}

int main(int argc, char *argv[]) {
	pid_t child;
	struct winsize size;
	const char *debug_file, *env_term;
	struct termios child_termios;
	int master;
	

	opt_init(&g.conf);
	if(opt_parse(argc, argv, &g.conf) != 0) {
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

	if(tcgetattr(STDIN_FILENO, &child_termios) != 0) {
		err_exit(errno, "tcgetattr failed");
	}

	if(g.conf.debug_file != NULL)
		debug_file = g.conf.debug_file;
	else
		debug_file = "/dev/null";
		
	if(freopen(debug_file, "w", stderr) == NULL) {
		err_exit(errno, "redirect stderr");
	}
		
	setlocale(LC_ALL,"");
	
	env_term = getenv("TERM"); 
	/* set the PARENT terminal profile to --ncterm if supplied.
	 * have to set this before calling screen_init to affect ncurses */
	if(g.conf.ncterm != NULL)
		if(setenv("TERM", g.conf.ncterm, 1) != 0)
			err_exit(errno, "error setting environment variable: %s", g.conf.ncterm);


	/* block winch right off the bat
	 * because we want to defer 
	 * processing of it until 
	 * curses and vterm are set up
	 */
	block_winch();
	g.winch_act.sa_handler = handler_winch;
	sigemptyset(&g.winch_act.sa_mask);
	g.winch_act.sa_flags = 0;

	/* screen will resize term to the right size,
	 * so just initialize to 1x1. */
	term_init(&g.term, 1, 1);
	if(screen_init(&g.term) != 0)
		err_exit(0, "screen_init failure");
	err_exit_cleanup_fn(err_exit_cleanup);
	
	term_dims(&g.term, (int *) &size.ws_row, (int *) &size.ws_col);

	if(g.conf.nocolor == 0)
		if(screen_color_start() != 0) {
			printf("failed to start color\n");
			goto cleanup;
		}

	child = forkpty(&master, NULL, &child_termios, &size);
	if(child == 0) {
		const char *child_term;
		/* set terminal profile for CHILD to supplied --term arg if exists 
		 * otherwise make sure to reset the TERM variable to the initial
		 * environment, even if it was null.
		 */
		child_term = NULL;
		if(g.conf.term != NULL) 
			child_term = g.conf.term;
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
		execvp(g.conf.cmd_argv[0], (char *const *) g.conf.cmd_argv);
		err_exit(errno, "cannot exec %s", g.conf.cmd_argv[0]);
	}

	if(set_nonblocking(master) != 0)
		err_exit(errno, "failed to set master fd to non-blocking");
	if(term_set_master(&g.term, master) != 0)
		err_exit(errno, "failed to set master pty in term structure");
	if(sigaction(SIGWINCH, &g.winch_act, &g.prev_winch_act) != 0)
		err_exit(errno, "sigaction failed");
	
	loop(&g.term);

cleanup:
	term_free(&g.term);
	screen_free();

	return 0;
}
