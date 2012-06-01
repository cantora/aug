#ifndef AUG_NCURSES_TEST_H
#define AUG_NCURSES_TEST_H

#include <stdio.h>
#include <stdarg.h>

#if defined(__APPLE__)
#	include <ncurses.h>
#else
#	include <ncursesw/curses.h>
#endif

#include "util.h"

#ifndef NCT_USE_SCREEN
static int nct_pipe_fds[2];
static int nct_fds_initialized = 0;

static void ncurses_test_init_pipe() {
	nct_fds_initialized = 1;

	AUG_STATUS_EQUAL( pipe(nct_pipe_fds), 0 );
}
#endif

static WINDOW *ncurses_test_init(const char *path) {

#ifdef NCT_USE_SCREEN
	(void)(name);
	return initscr();
#else
	SCREEN *scr;
	FILE *nct_out;
	
	if(nct_fds_initialized == 0)
		ncurses_test_init_pipe();

	AUG_PTR_NON_NULL( (nct_out = fopen( ( (path == NULL)? "/dev/null" : path ) , "w")) );
	
	if(dup2(nct_pipe_fds[0], 0) == -1) 
		err_exit(errno, "error duping to stdin");

	AUG_PTR_NON_NULL( (scr = newterm(getenv("TERM"), nct_out, stdin) ) );

	return stdscr;
#endif
}


static int ncurses_test_end() {
#ifdef NCT_USE_SCREEN
	close(nct_pipe_fds[1]);
#endif
	endwin();
	return OK;
}

static int nct_printf(const char *format, ...) {
#ifdef NCT_USE_SCREEN
	(void)(format);
	return 0;
#else
	va_list args;
	int result;

	va_start(args, format);
	result = dprintf(nct_pipe_fds[1], format, args);
	va_end(args);

	return result;
#endif
}

#endif /* AUG_NCURSES_TEST_H */