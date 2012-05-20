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

#include "screen.h"
#include <errno.h>
#include <stdint.h>
#include <assert.h>

#if defined(__APPLE__)
#	include <ncurses.h>
#else
#	include <ncursesw/curses.h>
#endif

#include "util.h"
#include "err.h"
#include "attr.h"

static int term_win_free();
static int term_win_init();
static void update_cell(VTermScreen *vts, VTermPos pos);

/* globals */
static struct {
	VTerm *vt;
	int color_on;
	WINDOW *term_win;
} g;	


static inline int win_contained(WINDOW *win, int y, int x) {
	int maxx, maxy;

	getmaxyx(win, maxy, maxx);	
	return (x < maxx) && (y < maxy);
}

int screen_init() {
	g.vt = NULL;
	g.color_on = 0;
	g.term_win = NULL;
	
	initscr();
	if(raw() == ERR) 
		goto fail;
	if(noecho() == ERR) 
		goto fail;
	if(term_win_init() != 0)
		goto fail;
	if(nodelay(g.term_win, true) == ERR) 
		goto fail;
	if(keypad(g.term_win, false) == ERR) /* libvterm interprets keys for us */
		goto fail;
	if(nonl() == ERR) 
		goto fail;
	
	return 0;
fail:
	errno = SCN_ERR_INIT;
	return -1;
}

static int term_win_free() {
	if(g.term_win != NULL)
		if(delwin(g.term_win) == ERR)
			return -1;

	return 0;	
}

static int term_win_init() {

	if(term_win_free() != 0)
		goto fail;

	g.term_win = newwin(LINES, COLS, 0, 0);

	if(g.term_win == NULL)
		goto fail;

	return 0;
fail:
	return -1;
}

void screen_free() {
	g.vt = NULL;

	if(term_win_free() != 0)
		err_exit(0, "term_win_free failed!");
	g.term_win = NULL;

	if(endwin() == ERR)
		err_exit(0, "endwin failed!");
}

void screen_set_term(VTerm *term) {
	g.vt = term;
}

void screen_dims(unsigned short *rows, unsigned short *cols) {
	*rows = LINES;
	*cols = COLS;
}

void screen_term_win_dims(unsigned short *rows, unsigned short *cols) {
	getmaxyx(g.term_win, *rows, *cols);
}

int screen_getch(int *ch) {
	*ch = wgetch(g.term_win);

	/* resize will be handled by signal
	 * so flush out all the resize keys
	 */
	if(*ch == KEY_RESIZE) 
		while(*ch == KEY_RESIZE) {
			*ch = wgetch(g.term_win);
		}

	if(*ch == ERR) 
		return -1;

	return 0;
}

/*void screen_err_msg(int error, char **msg) {
	switch(error) {
	case SCN_ERR_NONE:
		*msg = "no error";
		break;
	case SCN_ERR_INIT:
		*msg = "failed to initialize ncurses";
		break;
	case SCN_ERR_PAIRS:
		*msg = "terminal with more than SCN_REQ_PAIRS required";
		break;
	default:
		*msg = "unknown error";	
	}
} */

int screen_color_start() {
	int colors[] = { COLOR_DEFAULT, COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
						 COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE };
	size_t i, k;
	int fg, bg, pair;
		
	if(has_colors() == ERR) {
		errno = SCN_ERR_COLOR_NOT_SUPPORTED;
		goto fail;
	}

	if(start_color() == ERR) {
		errno = SCN_ERR_COLOR_NOT_SUPPORTED;
		goto fail;
	}
	
	if(use_default_colors() == ERR) {
		errno = SCN_ERR_COLOR_NO_DEFAULT;
		goto fail;		
	}
	
	if(COLORS < AUG_REQ_COLORS-1) {
		errno = SCN_ERR_COLOR_COLORS;
		goto fail;
	}

	/* for some reason we can just use pairs above 63 and its fine... */
	if(COLOR_PAIRS < AUG_REQ_PAIRS) {
		/*errno = AUG_ERR_COLOR_PAIRS;
		goto fail;*/
		fprintf(stderr, "warning: your terminal may not support 81 color pairs. if problems arise, try setting TERM to 'xterm-256color'\n");
	}

	for(i = 0; i < AUG_ARRAY_SIZE(colors); i++) {
		fg = colors[i];
		for(k = 0; k < AUG_ARRAY_SIZE(colors); k++) {
			bg = colors[k];
			attr_curses_colors_to_curses_pair(fg, bg, &pair);
			/* pair 0 is default on default and is already defined */
			if(pair == 0) { 
				continue;
			}
						
			if(init_pair(pair, fg, bg) == ERR) {
				errno = SCN_ERR_COLOR_PAIR_INIT;
				goto fail;
			}
		}
	}

	g.color_on = 1;
	return 0;
fail:
	return -1;
}

static void update_cell(VTermScreen *vts, VTermPos pos) {
	VTermScreenCell cell;
	attr_t attr;
	int pair;
	cchar_t cch;
	wchar_t *wch;
	wchar_t erasech = L' ';
	int maxx, maxy;

	getmaxyx(g.term_win, maxy, maxx);

	/* sometimes this happens when
	 * a window resize recently happened
	 */
	if(!win_contained(g.term_win, pos.row, pos.col) ) {
		fprintf(stderr, "tried to update out of bounds cell at %d/%d %d/%d\n", pos.row, maxy-1, pos.col, maxx-1);
		return;
	}

	vterm_screen_get_cell(vts, pos, &cell);	
	attr_vterm_attr_to_curses_attr(&cell, &attr);
	if(g.color_on)
		attr_vterm_pair_to_curses_pair(cell.fg, cell.bg, &attr, &pair);
	else
		pair = 0;

	
	wch = (cell.chars[0] == 0)? &erasech : (wchar_t *) &cell.chars[0];
	if(setcchar(&cch, wch, attr, pair, NULL) == ERR)
		err_exit(0, "setcchar failed");

	if(wmove(g.term_win, pos.row, pos.col) == ERR)
		err_exit(0, "move failed: %d/%d, %d/%d\n", pos.row, maxy-1, pos.col, maxx-1);

	if(wadd_wch(g.term_win, &cch) == ERR && pos.row != (maxy-1) && pos.col != (maxx-1) )
		err_exit(0, "add_wch failed at %d/%d, %d/%d: ", pos.row, maxy-1, pos.col, maxx-1);

}

void screen_refresh() {
	if(wrefresh(g.term_win) == ERR)
		err_exit(0, "wrefresh failed!");
}

int screen_damage(VTermRect rect, void *user) {
	VTermScreen *vts = vterm_obtain_screen(g.vt);
	VTermPos pos;
	int x,y,maxx,maxy;

	(void)(user); /* user not used */

	/* save cursor value */
	getyx(g.term_win, y, x);
	getmaxyx(g.term_win, maxy, maxx);

	for(pos.row = rect.start_row; pos.row < rect.end_row; pos.row++) {
		for(pos.col = rect.start_col; pos.col < rect.end_col; pos.col++) {
			update_cell(vts, pos);
		}
	}

	/* restore cursor (repainting shouldnt modify cursor) */
	if(wmove(g.term_win, y,x) == ERR) 
		err_exit(0, "move failed: %d/%d %d/%d", y, maxy, x, maxx);

	/*fprintf(stderr, "\tdamage: (%d,%d) (%d,%d) \n", rect.start_row, rect.start_col, rect.end_row, rect.end_col);*/

	return 1;
}

/*
int screen_moverect(VTermRect dest, VTermRect src, void *user) {
	
	//err_exit(0, "moverect called: dest={%d,%d,%d,%d}, src={%d,%d,%d,%d} (%d,%d)", dest.start_row, dest.start_col, dest.end_row, dest.end_col, src.start_row, src.start_col, src.end_row, src.end_col, maxy, maxx);
	return 0;
}*/

int screen_movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user) {
	int maxx, maxy;
	(void)(user); /* user is not used */
	(void)(oldpos); /* oldpos not used */
	(void)(visible);

	getmaxyx(g.term_win, maxy, maxx);
		
	/* sometimes this happens when
	 * a window resize recently happened
	 */
	if(!win_contained(g.term_win, pos.row, pos.col) ) {
		fprintf(stderr, "tried to move cursor out of bounds to %d/%d %d/%d\n", pos.row, maxy-1, pos.col, maxx-1);
		return 1;
	}

	if(wmove(g.term_win, pos.row, pos.col) == ERR)
		err_exit(0, "move failed: %d/%d %d/%d", pos.row, maxy-1, pos.col, maxx-1);

	/*fprintf(stderr, "\tmove cursor: %d, %d\n", pos.row, pos.col);*/

	return 1;
}

int screen_bell(void *user) {
	(void)(user);

	if(beep() == ERR) {
		fprintf(stderr, "bell failed\n");
	}

	/*if(flash() == ERR) {
		fprintf(stderr, "flash failed\n");
	}*/
	return 1;
}

int screen_settermprop(VTermProp prop, VTermValue *val, void *user) {
	
	(void)(user);

	/*fprintf(stderr, "settermprop: %d", prop);*/
	switch(prop) { 
	case VTERM_PROP_CURSORVISIBLE:
		/* fprintf(stderr, " (CURSORVISIBLE) = %02x", val->boolean); */
		/* will return ERR if cursor not supported, *
		 * so we dont bother checking return value  */
		curs_set(!!val->boolean); 
		break;
	case VTERM_PROP_CURSORBLINK: /* not sure if ncurses can change blink settings */
		/* fprintf(stderr, " (CURSORBLINK) = %02x", val->boolean); */
		break;
	case VTERM_PROP_REVERSE: /* this should be taken care of by update cell i think */
		/* fprintf(stderr, " (REVERSE) = %02x", val->boolean); */
		break;
	case VTERM_PROP_CURSORSHAPE: /* dont think curses can change cursor shape */
		/* fprintf(stderr, " (CURSORSHAPE) = %d", val->number); */
		break;
	default:
		;
	}

	/* fprintf(stderr, "\n"); */

	return 1;
}

/* this should be called before vterm_set_size
 * which will cause the term damage callback
 * to fully rewrite the screen.
 */
void screen_resize() {
	if(endwin() == ERR)
		err_exit(0, "endwin failed!");

	/*if(refresh() == ERR)
		err_exit(0, "refresh failed!");*/

	screen_refresh();	
}
