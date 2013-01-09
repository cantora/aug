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

#define AUG_CHILD_BUF_SIZE 2048*4

struct aug_child {
	char buf[AUG_CHILD_BUF_SIZE];
	struct aug_term *term;	
	AUG_LOCK_MEMBERS;
};

void child_init(struct aug_child *child, struct aug_term *term, 
		char *const *cmd_argv, void (*exec_cb)(),
		struct termios *child_termios);

void child_io_loop(struct aug_child *child, int fd_input, void (*to_lock)(), 
		void (*to_unlock)(), void (*to_refresh)(), 
		void (*to_process_input)(struct aug_term *term, int fd_input) );

#endif /* AUG_CHILD_H */
