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
#ifndef AUG_CHILD_H
#define AUG_CHILD_H

#include <unistd.h>
#if defined(__FreeBSD__)
#	include <libutil.h>
#	include <termios.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#	include <termios.h>
#	include <util.h>
#else
#	include <pty.h>
#endif

#include "lock.h"
#include "timer.h"

#define AUG_CHILD_READ_SIZE 4096
#define AUG_CHILD_BUF_SIZE (AUG_CHILD_READ_SIZE*4)

struct aug_child {
	char buf[AUG_CHILD_BUF_SIZE];
	struct aug_term *term;	
	AUG_LOCK_MEMBERS;
	pid_t pid;
	void (*to_refresh)(void *);
	void (*to_lock)(void *);
	void (*to_unlock)(void *);
	struct aug_timer refresh_min;
	int got_input;
	void *user;
};

void child_init(struct aug_child *child, struct aug_term *term, 
		char *const *cmd_argv, void (*exec_cb)(),
		void (*to_refresh)(void *), void (*to_lock)(void *), 
		void (*to_unlock)(void *), struct termios *child_termios,
		void *user);
void child_free(struct aug_child *child);
void child_got_input(struct aug_child *child);
void child_io_loop(struct aug_child *child, int fd_input, 
		int (*to_process_input)(struct aug_term *term, int fd_input, void *) );
void child_process_term_output(struct aug_child *child);
void child_lock(struct aug_child *child);
void child_unlock(struct aug_child *child);
void child_refresh(struct aug_child *child);

#endif /* AUG_CHILD_H */
