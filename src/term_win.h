#ifndef AUG_TERM_WIN
#define AUG_TERM_WIN

#if defined(__APPLE__)
#	include <ncurses.h>
#else
#	include <ncursesw/curses.h>
#endif

#include "term.h"

struct aug_term_win_t {
	WINDOW *win;
	struct aug_term_t *term;	
};


#endif /* AUG_TERM_WIN */