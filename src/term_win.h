#ifndef AUG_TERM_WIN
#define AUG_TERM_WIN

#if defined(__APPLE__)
#	include <ncurses.h>
#else
#	include <ncursesw/curses.h>
#endif

#include "term.h"

struct aug_term_win {
	WINDOW *win;
	struct aug_term *term;	
};

void term_win_init(struct aug_term_win *tw, WINDOW *win);
void term_win_set_term(struct aug_term_win *tw, struct aug_term *term);
void term_win_dims(const struct aug_term_win *tw, int *rows, int *cols);
void term_win_update_cell(struct aug_term_win *tw, VTermPos pos, int color_on);
void term_win_refresh(struct aug_term_win *tw);
int term_win_damage(struct aug_term_win *tw, VTermRect rect, int color_on);
int term_win_movecursor(struct aug_term_win *tw, VTermPos pos);
void term_win_resize(struct aug_term_win *tw);

#endif /* AUG_TERM_WIN */