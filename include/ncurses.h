#ifndef AUG_NCURSES_H
#define AUG_NCURSES_H

#ifndef _XOPEN_SOURCE
#	define _XOPEN_SOURCE 700
#endif

#ifndef _XOPEN_SOURCE_EXTENDED
#	define _XOPEN_SOURCE_EXTENDED 1
#endif

#if defined(__APPLE__)
#	include <ncurses.h>
#else
#	include <ncursesw/curses.h>
#endif

#endif /* AUG_NCURSES_H */