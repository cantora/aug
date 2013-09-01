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
#include "child.h"

#include <sys/select.h>
#include <errno.h>

#include "err.h"
#include "util.h"
#include "term.h"

#ifdef AUG_DEBUG_IO
#	define AUG_DEBUG_IO_LOG(...) \
		fprintf(stderr, __VA_ARGS__)

#	define AUG_DEBUG_IO_TIME_MIN 10000
#else
#	define AUG_DEBUG_IO_LOG(...)
#endif

static int process_master_output(struct aug_child *);

void child_init(struct aug_child *child, struct aug_term *term, 
		char *const *cmd_argv, void (*exec_cb)(),
		void (*to_refresh)(void *), void (*to_lock)(void *), 
		void (*to_unlock)(void *), struct termios *child_termios,
		void *user) {
	struct winsize size;
	int master;

	memset(&size, 0, sizeof(size)); /* to make valgrind happy */
	child->pid = forkpty(&master, NULL, child_termios, &size);
	if(child->pid == 0) {
		(*exec_cb)();
		execvp(cmd_argv[0], cmd_argv);
		err_exit(errno, "cannot exec %s", cmd_argv[0]);
	}

	if(set_nonblocking(master) != 0)
		err_exit(errno, "failed to set master fd to non-blocking");
	if(term_set_master(term, master) != 0)
		err_exit(errno, "failed to set master pty in term structure");

	child->term = term;
	child->to_refresh = to_refresh;
	child->to_lock = to_lock;
	child->to_unlock = to_unlock;
	child->got_input = 0;
	child->user = user;
	AUG_LOCK_INIT(child);
}

void child_free(struct aug_child *child) {
	AUG_LOCK_FREE(child);
}

void child_process_term_output(struct aug_child *child) {
	size_t buflen;
#ifdef AUG_DEBUG_IO
	AUG_TIMER_ALLOC();
	AUG_TIMER_START();
#endif

	while( (buflen = vterm_output_get_buffer_current(child->term->vt) ) > 0) {
		buflen = (buflen < AUG_CHILD_BUF_SIZE)? buflen : AUG_CHILD_BUF_SIZE;
		buflen = vterm_output_bufferread(child->term->vt, child->buf, buflen);
		write_n_or_exit(child->term->master, child->buf, buflen);
	}

#ifdef AUG_DEBUG_IO
	AUG_TIMER_IF_EXCEEDED(0, AUG_DEBUG_IO_TIME_MIN) {
		AUG_TIMER_DISPLAY(stderr, "writing to child->term->master took %d,%d secs\n");
	}
#endif
}

static int process_master_output(struct aug_child *child) {
	ssize_t n_read, total_read;
#ifdef AUG_DEBUG_IO
	AUG_TIMER_ALLOC();
	AUG_TIMER_START();
#endif

	total_read = 0;
	/*fprintf(stderr, "child: read master pty\n");*/
	do {
		n_read = read(child->term->master, child->buf + total_read, AUG_CHILD_READ_SIZE);
		/*fprintf(stderr, "child: read %zd from master pty\n", n_read);*/
	} while(n_read > 0 && ( (total_read += n_read) + AUG_CHILD_READ_SIZE <= AUG_CHILD_BUF_SIZE) );
	/*fprintf(stderr, "child: done reading master pty\n");*/

#ifdef AUG_DEBUG_IO
	AUG_TIMER_IF_EXCEEDED(0, AUG_DEBUG_IO_TIME_MIN) {
		AUG_TIMER_DISPLAY(stderr, "reading from child->term->master took %d,%d secs\n");
	}
#endif
	
	if(n_read == 0 || (n_read < 0 && errno == EIO) ) { 
		/* potential issue: if total_read > 0 should we still
		 * bother writing those bytes to the terminal even
		 * though we are presumably about to close whatever
		 * was displaying the terminal output? */
		return -1; /* the master pty is closed, return -1 to signify */
	}
	else if(n_read < 0 && errno != EAGAIN) {
		err_exit(errno, "error reading from pty master (n_read = %d)", n_read);
	}
	/* else errno == EAGAIN: we dont need to do anything except 
	 * output the contents of g_buf if total_read > 0 
	 */
	
	if(total_read > 0) {
#ifdef AUG_DEBUG_IO
		AUG_TIMER_START();
#endif
		vterm_push_bytes(child->term->vt, child->buf, total_read);
#ifdef AUG_DEBUG_IO
		AUG_TIMER_IF_EXCEEDED(0, AUG_DEBUG_IO_TIME_MIN) {
			AUG_TIMER_DISPLAY(stderr, "vterm_push_bytes took %d,%d secs\n");
		}
#endif
	}
	
	return 0;
}

void child_got_input(struct aug_child *child) {
	child->got_input = 1;
}

/* this function calls the -to_unlock- callback first, thus it 
 * expects all resources to be in a locked state when entering
 * this function.
 */
void child_io_loop(struct aug_child *child, int fd_input, 
		int (*to_process_input)(struct aug_term *term, int fd_input, void *) ) {
	fd_set in_fds;
	int status, high_fd, force_refresh, just_refreshed;
	struct timeval tv_select;
	struct timeval *tv_select_p;
#ifdef AUG_DEBUG_IO
	AUG_TIMER_ALLOC();
#endif

	if(child->term->master < 0) 
		err_exit(0, "invalid master fd: %d", child->term->master);

	timer_init(&child->refresh_min);

	/* when we have just refreshed we can use a blocking select
	 * call (and ignore refresh_max) because we have no need to
	 * refresh unless some I/O happens. */
	just_refreshed = 0;
	child->got_input = 0;
	force_refresh = 1;

	fprintf(stderr, "fd_input = %d\n", fd_input);	
	high_fd = (child->term->master > fd_input)? child->term->master : fd_input;

	while(1) {
		/* if master pty is 'bursting' with I/O at a quick rate
		 * we want to let the burst finish 
		 * and then refresh the screen, otherwise we waste a bunch of time
		 * refreshing the screen with stuff that just gets scrolled off
		 */
		if(force_refresh != 0
				|| (status = timer_thresh(&child->refresh_min, 0, 71400) ) == 1 ) {
			/* refresh at most 14 times per second (assuming there
			 * is anything at all to refresh). */
			child_refresh(child);
			just_refreshed = 1;
			force_refresh = 0;
		}
		else if(status < 0)
			err_exit(errno, "timer error");
		else
			just_refreshed = 0; /* didnt refresh the screen on this iteration */

		FD_ZERO(&in_fds);
		if(fd_input >= 0)
			FD_SET(fd_input, &in_fds);
		FD_SET(child->term->master, &in_fds);

		/* if we just refreshed the screen there
		 * is no need to timeout the select wait. 
		 * however if we havent refreshed on the last
		 * iteration we need to make sure that inter_io_timer
		 * is given a chance to timeout and cause a refresh.
		 */
		tv_select.tv_sec = 0;
		tv_select.tv_usec = 15000;
		tv_select_p = (just_refreshed == 0)? &tv_select : NULL;

		child_unlock(child);
	
		AUG_DEBUG_IO_LOG("child: select begin\n");
#ifdef AUG_DEBUG_IO
		AUG_TIMER_START();
#endif
		if(select(high_fd+1, &in_fds, NULL, NULL, tv_select_p) == -1) {
			if(errno == EINTR) {
				AUG_DEBUG_IO_LOG("child: select interupted\n");
#ifdef AUG_DEBUG_IO
				AUG_TIMER_IF_EXCEEDED(0, AUG_DEBUG_IO_TIME_MIN) {
					AUG_TIMER_DISPLAY(stderr, "select took %d,%d secs\n");
				}
#endif				
				child_lock(child);
				continue;
			}
			else
				err_exit(errno, "select");
		}
		AUG_DEBUG_IO_LOG("child: select end\n");
#ifdef AUG_DEBUG_IO
		AUG_TIMER_IF_EXCEEDED(0, AUG_DEBUG_IO_TIME_MIN) {
			AUG_TIMER_DISPLAY(stderr, "select took %d,%d secs\n");
		}
#endif				
		child_lock(child);

		if(FD_ISSET(child->term->master, &in_fds) ) {
			AUG_DEBUG_IO_LOG("child: process_master_output\n");
			if(process_master_output(child) != 0) {
				break;
			}
			child_process_term_output(child);

			/* if we just got input, we might get a character
			 * echoed back by a shell, so we want to force a
			 * refresh so that the echo comes out as fast as
			 * possible */
			if(child->got_input != 0)
				force_refresh = 1;
		}

		child->got_input = 0;
		if(fd_input >= 0 && FD_ISSET(fd_input, &in_fds) ) {
			AUG_DEBUG_IO_LOG("child: process input\n");
			if( (*to_process_input)(child->term, fd_input, child->user) != 0 ) {
				/* fd_input is closed or bad in some way */
				break;
			}			
			child_process_term_output(child);
			force_refresh = 1;
			child_got_input(child);
		} /* if input fd set */
	} /* while(1) */

	child_unlock(child);
	return;
}

void child_lock(struct aug_child *child) {
	AUG_LOCK(child);
	(*child->to_lock)(child->user);
}

void child_unlock(struct aug_child *child) { 
	(*child->to_unlock)(child->user);
	AUG_UNLOCK(child);
}

void child_refresh(struct aug_child *child) {
#ifdef AUG_DEBUG_IO
	AUG_TIMER_ALLOC();
#endif

	AUG_DEBUG_IO_LOG("child: refresh\n");

#ifdef AUG_DEBUG_IO
	AUG_TIMER_START();
#endif
	vterm_screen_flush_damage(vterm_obtain_screen(child->term->vt) );
#ifdef AUG_DEBUG_IO
	AUG_TIMER_IF_EXCEEDED(0, AUG_DEBUG_IO_TIME_MIN) {
		AUG_TIMER_DISPLAY(stderr, "vterm_screen_flush_damage took %d,%d secs\n");
	}
#endif

#ifdef AUG_DEBUG_IO
	AUG_TIMER_START();
#endif
	if(child->term->io_callbacks.refresh != NULL)
		(*child->term->io_callbacks.refresh)(child->term->user); /* call the term refresh callback */
#ifdef AUG_DEBUG_IO
	AUG_TIMER_IF_EXCEEDED(0, AUG_DEBUG_IO_TIME_MIN) {
		AUG_TIMER_DISPLAY(stderr, "child->term->io_callbacks.refresh took %d,%d secs\n");
	}
#endif

#ifdef AUG_DEBUG_IO
	AUG_TIMER_START();
#endif
	(*child->to_refresh)(child->user);
#ifdef AUG_DEBUG_IO
	AUG_TIMER_IF_EXCEEDED(0, AUG_DEBUG_IO_TIME_MIN) {
		AUG_TIMER_DISPLAY(stderr, "child->to_refresh took %d,%d secs\n");
	}
#endif
	
	timer_init(&child->refresh_min);
}
