#include "child.h"

#include <sys/select.h>
#include <errno.h>

#include "timer.h"
#include "err.h"
#include "util.h"
#include "term.h"

static void process_vterm_output(struct aug_child *);
static int process_master_output(struct aug_child *);

void child_init(struct aug_child *child, struct aug_term *term, 
		char *const *cmd_argv, void (*exec_cb)(),
		struct termios *child_termios) {
	struct winsize size;
	int master;

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
	AUG_LOCK_INIT(child);
}

static void process_vterm_output(struct aug_child *child) {
	size_t buflen;

	while( (buflen = vterm_output_get_buffer_current(child->term->vt) ) > 0) {
		buflen = (buflen < AUG_CHILD_BUF_SIZE)? buflen : AUG_CHILD_BUF_SIZE;
		buflen = vterm_output_bufferread(child->term->vt, child->buf, buflen);
		write_n_or_exit(child->term->master, child->buf, buflen);
	}
}

static int process_master_output(struct aug_child *child) {
	ssize_t n_read, total_read;

	total_read = 0;
	/* as far as i have seen, linux will not generally 
	 * return more than 512 bytes from a read, even if more
	 * than 512 is available, so by looping here, we try
	 * to get as close to filling up the buffer as possible */
	do {
		n_read = read(child->term->master, child->buf + total_read, 512);
	} while(n_read > 0 && ( (total_read += n_read) + 512 <= AUG_CHILD_BUF_SIZE) );
	
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
	
	if(total_read > 0)
		vterm_push_bytes(child->term->vt, child->buf, total_read);
	
	return 0;
}

/* this function calls the -to_unlock- callback first, thus it 
 * expects all resources to be in a locked state when entering
 * this function.
 */
void child_io_loop(struct aug_child *child, int fd_input, void (*to_lock)(), 
		void (*to_unlock)(), void (*to_refresh)(), 
		void (*to_process_input)(struct aug_term *term, int fd_input) ) {
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
		FD_SET(fd_input, &in_fds);
		FD_SET(child->term->master, &in_fds);

		/* if we just refreshed the screen there
		 * is no need to timeout the select wait. 
		 * however if we havent refreshed on the last
		 * iteration we need to make sure that inter_io_timer
		 * is given a chance to timeout and cause a refresh.
		 */
		tv_select.tv_sec = 0;
		tv_select.tv_usec = 1000;
		tv_select_p = (just_refreshed == 0)? &tv_select : NULL;

		(*to_unlock)();

		if(select(child->term->master + 1, &in_fds, NULL, NULL, tv_select_p) == -1) {
			if(errno == EINTR) {
				(*to_lock)();
				continue;
			}
			else
				err_exit(errno, "select");
		}		
		(*to_lock)();

		if(FD_ISSET(child->term->master, &in_fds) ) {
			if(process_master_output(child) != 0) {
				goto done;
			}

			process_vterm_output(child);
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
			if(child->term->io_callbacks.refresh != NULL)
				(*child->term->io_callbacks.refresh)(child->term->user); /* call the term refresh callback */

			(*to_refresh)();
			
			timer_init(&inter_io_timer);
			timer_init(&refresh_expire);
			force_refresh = 0;
			just_refreshed = 1;
		}
		else if(status < 0)
			err_exit(errno, "timer error");
		else
			just_refreshed = 0; /* didnt refresh the screen on this iteration */

		if(FD_ISSET(fd_input, &in_fds) ) {
			(*to_process_input)(child->term, fd_input); 
			process_vterm_output(child);
			force_refresh = 1;
		} /* if stdin set */
	} /* while(1) */

done:
	(*to_unlock)();
	return;
}

