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
#ifndef AUG_NCURSES_TEST_H
#define AUG_NCURSES_TEST_H

#ifndef _XOPEN_SOURCE
#	define _XOPEN_SOURCE 700
#endif

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#if defined(__APPLE__)
#	include <ncurses.h>
#else
#	include <ncurses.h>
#endif

#include "util.h"

static int nct_pipe_fds[2];
static int nct_fds_initialized = 0;

static void ncurses_test_init_pipe() {
	nct_fds_initialized = 1;

	AUG_STATUS_EQUAL( pipe(nct_pipe_fds), 0 );
}

static WINDOW *ncurses_test_init(const char *path) {
	SCREEN *scr;
	FILE *nct_out;
	
	if(nct_fds_initialized == 0)
		ncurses_test_init_pipe();

	if(path == NULL) {
		nct_out = stdout;
	}
	else {
		AUG_PTR_NON_NULL( (nct_out = fopen(path , "w")) );
	}

	if(dup2(nct_pipe_fds[0], 0) == -1) 
		err_exit(errno, "error duping to stdin");

	AUG_PTR_NON_NULL( (scr = newterm(getenv("TERM"), nct_out, stdin) ) );

	return stdscr;
}


static int ncurses_test_end() {
	endwin();
	return OK;
}

static int nct_printf(const char *format, ...) {
	va_list args;
	int result, i;
	char buf[1024];
	
	va_start(args, format);
	result = vsnprintf(buf, 1024, format, args);
	va_end(args);

	if(result < 0)
		err_exit(0, "vsnprintf failed");

	for(i = 0; i < result; i++) {
		if(write(nct_pipe_fds[1], buf + i, 1) != 1)
			err_exit(0, "error writing to nct pipe");

		usleep(50000);
	}

	return result;
}

#endif /* AUG_NCURSES_TEST_H */