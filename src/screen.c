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
#include <string.h>

#include "ncurses.h"

#include "util.h"
#include "err.h"
#include "attr.h"
#include "term_win.h"

static void vterm_cb_refresh(void *user);
static int free_term_win();
static int init_term_win();

static const VTermScreenCallbacks CB_SCREEN = {
	.damage = screen_damage,
	.movecursor = screen_movecursor,
	.bell = screen_bell,
	.settermprop = screen_settermprop
};

static const struct aug_term_io_callbacks CB_TERM_IO = {
	.refresh = vterm_cb_refresh
};

/* globals */
static struct {
	int color_on;
	struct aug_term_win term_win;
} g;	

int screen_init(struct aug_term *term) {
	g.color_on = 0;

	initscr();
	if(raw() == ERR) 
		goto fail;
	if(noecho() == ERR) 
		goto fail;

	if(init_term_win() != 0)
		goto fail;
	screen_set_term(term);

	if(nodelay(stdscr, true) == ERR) 
		goto fail;
	if(nonl() == ERR)
		goto fail;
	
	return 0;
fail:
	errno = SCN_ERR_INIT;
	return -1;
}

static int free_term_win() {
	if(g.term_win.win != NULL)
		if(delwin(g.term_win.win) == ERR)
			return -1;

	return 0;
}

static int init_term_win() {
	WINDOW *win;

	if(free_term_win() != 0)
		goto fail;

	win = derwin(stdscr, LINES, COLS, 0, 0);
	if(win == NULL)
		goto fail;

	term_win_init(&g.term_win, win);

	return 0;
fail:
	return -1;
}

int screen_cleanup() {
	if(endwin() == ERR) {
		return -1;
	}
	else {
		return 0;
	}
}

void screen_free() {
	if(free_term_win() != 0)
		err_exit(0, "free_term_win failed!");

	if(screen_cleanup() != 0)
		err_exit(0, "screen_cleanup failed!");
}

static void vterm_cb_refresh(void *user) {
	(void)user;

	term_win_refresh(&g.term_win);
}

void screen_set_term(struct aug_term *term) {
	if(g.term_win.term != NULL)
		term_clear_callbacks(g.term_win.term);
	
	term_set_callbacks(term, &CB_SCREEN, &CB_TERM_IO, NULL);
	term_win_set_term(&g.term_win, term);	
}

void screen_dims(int *rows, int *cols) {
	*rows = LINES;
	*cols = COLS;
}

int screen_getch(int *ch) {
	*ch = getch();

	/* resize will be handled by signal
	 * so flush out all the resize keys
	 */
	if(*ch == KEY_RESIZE) 
		while(*ch == KEY_RESIZE) {
			*ch = getch();
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

void screen_refresh() {
	if(refresh() == ERR) 
		err_exit(0, "refresh failed");
}

int screen_damage(VTermRect rect, void *user) {
	(void)(user);

	return term_win_damage(&g.term_win, rect, g.color_on);
}

/*
int screen_moverect(VTermRect dest, VTermRect src, void *user) {
	
	//err_exit(0, "moverect called: dest={%d,%d,%d,%d}, src={%d,%d,%d,%d} (%d,%d)", dest.start_row, dest.start_col, dest.end_row, dest.end_col, src.start_row, src.start_col, src.end_row, src.end_col, maxy, maxx);
	return 0;
}*/

int screen_movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user) {
	(void)(visible);
	(void)(user);

	return term_win_movecursor(&g.term_win, pos, oldpos);
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
	
	screen_refresh();

	fprintf(stderr, "screen: resize to %d, %d\n", LINES, COLS);
	wresize(g.term_win.win, LINES, COLS);
	term_win_resize(&g.term_win);
}

/* converts a character into its string representation.
 * *str* should have enough space for 3 characters + one
 * one null byte.
 */
int screen_unctrl(int ch, char *str) {
	const char *s;
	size_t len;

	s = unctrl(ch);
	if(s == NULL)
		return -1;

	len = strlen(s);
	
	strncpy(str, s, 3);
	if(len > 3)
		len = 3;

	str[len] = '\0';

	return 0;
}

void screen_doupdate() {
	if(doupdate() == ERR) 
		err_exit(0, "doupdate failed");
}