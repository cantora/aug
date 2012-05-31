#ifndef AUG_NCURSES_TEST_H
#define AUG_NCURSES_TEST_H

#include <stdio.h>
#include <stdarg.h>
#include <ncurses.h>

#include "util.h"

#ifndef NCT_USE_SCREEN
static FILE *nct_out, *nct_in, *nct_to_in;
#endif

static void ncurses_test_init(const char *path) {

#ifdef NCT_USE_SCREEN
	(void)(name);
	initscr();
#else
	SCREEN *scr;
	int pd[2];
	
	AUG_PTR_NON_NULL( (nct_out = fopen( ( (path == NULL)? "/dev/null" : path ) , "w")) );
	AUG_STATUS_EQUAL( pipe(pd), 0 );
	AUG_PTR_NON_NULL( (nct_in = fdopen(pd[0], "r")) );
	AUG_PTR_NON_NULL( (nct_to_in = fdopen(pd[1], "w")) );

	AUG_PTR_NON_NULL( (scr = newterm(getenv("TERM"), nct_out, nct_in) ) );

	/*set_term(scr);*/
	
	/*if(def_prog_mode() == ERR)
		err_exit(0, "def_prog_mode failed");*/
#endif
}


static void ncurses_test_end() {
	endwin();

#ifndef NCT_USE_SCREEN
	fclose(nct_out);
	fclose(nct_in);
	fclose(nct_to_in);
#endif
}

static int nct_printf(const char *format, ...) {
#ifdef NCT_USE_SCREEN
	(void)(format);
	return 0;
#else
	va_list args;
	int result;

	va_start(args, format);
	result = vfprintf(nct_to_in, format, args);
	va_end(args);

	return result;
#endif
}

static int nct_flush() {
#ifdef NCT_USE_SCREEN
	return 0;
#else
	return fflush(nct_to_in);	
#endif
}

#endif /* AUG_NCURSES_TEST_H */