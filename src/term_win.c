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
#include "term_win.h"
#include <errno.h>
#include <stdint.h>
#include <assert.h>

#include "ncurses.h"

#include "util.h"
#include "err.h"
#include "attr.h"
#include "ncurses_util.h"

extern int aug_cell_update(int *row, int *col, wchar_t *wch, attr_t *attr, int *color_pair);
extern int aug_cursor_move(int old_row, int old_col, int *new_row, int *new_col);

void term_win_init(struct aug_term_win *tw, WINDOW *win) {
	tw->term = NULL;
	tw->win = win;
}

void term_win_set_term(struct aug_term_win *tw, struct aug_term *term) {
	tw->term = term;
	term_win_resize(tw);
}

void term_win_dims(const struct aug_term_win *tw, int *rows, int *cols) {
	win_dims(tw->win, rows, cols);
}

void term_win_update_cell(struct aug_term_win *tw, VTermPos pos, int color_on) {
	VTermScreen *vts;
	VTermScreenCell cell;
	attr_t attr;
	int pair;
	cchar_t cch;
	wchar_t *wch;
	wchar_t erasech = L' ';
	int maxx, maxy;

	if(tw->term == NULL)
		return;

	vts = vterm_obtain_screen(tw->term->vt);
	getmaxyx(tw->win, maxy, maxx);

	/* sometimes this happens when
	 * a window resize recently happened
	 */
	if(!win_contained(tw->win, pos.row, pos.col) ) {
		fprintf(stderr, "tried to update out of bounds cell at %d/%d %d/%d\n", pos.row, maxy-1, pos.col, maxx-1);
		return;
	}

	vterm_screen_get_cell(vts, pos, &cell);	
	attr_vterm_attr_to_curses_attr(&cell, &attr);
	if(color_on)
		attr_vterm_pair_to_curses_pair(cell.fg, cell.bg, &attr, &pair);
	else
		pair = 0;

	
	wch = (cell.chars[0] == 0)? &erasech : (wchar_t *) &cell.chars[0];

	if(aug_cell_update(&pos.row, &pos.col, wch, &attr, &pair) != 0) /* run API callbacks */
		return;
	if(setcchar(&cch, wch, attr, pair, NULL) == ERR)
		err_exit(0, "setcchar failed");
	if(wmove(tw->win, pos.row, pos.col) == ERR)
		err_exit(0, "move failed: %d/%d, %d/%d\n", pos.row, maxy-1, pos.col, maxx-1);

	/* sometimes writing to the last cell fails... but it doesnt matter? */
	if(wadd_wch(tw->win, &cch) == ERR && (pos.row) != (maxy-1) && (pos.col) != (maxx-1) )
		err_exit(0, "add_wch failed at %d/%d, %d/%d: ", pos.row, maxy-1, pos.col, maxx-1);

}

void term_win_refresh(struct aug_term_win *tw) {
	if(wrefresh(tw->win) == ERR)
		err_exit(0, "wrefresh failed!");
}

int term_win_damage(struct aug_term_win *tw, VTermRect rect, int color_on) {
	VTermPos pos;
	int x,y;

	/* save cursor value */
	getyx(tw->win, y, x);

	for(pos.row = rect.start_row; pos.row < rect.end_row; pos.row++) {
		for(pos.col = rect.start_col; pos.col < rect.end_col; pos.col++) {
			term_win_update_cell(tw, pos, color_on);
		}
	}

	/* restore cursor (repainting shouldnt modify cursor) */
	if(wmove(tw->win, y,x) == ERR) 
		err_exit(0, "move failed: %d, %d", y, x);

	return 1;
}

int term_win_movecursor(struct aug_term_win *tw, VTermPos pos, VTermPos oldpos) {

	/* sometimes this happens when
	 * a window resize recently happened. */
	 if(!win_contained(tw->win, pos.row, pos.col) ) {
		fprintf(stderr, "tried to move cursor out of bounds to %d, %d\n", pos.row, pos.col);
		return 1;
	}

	if(aug_cursor_move(oldpos.row, oldpos.col, &pos.row, &pos.col) != 0) /* run API callbacks */
		return 1;

	if(wmove(tw->win, pos.row, pos.col) == ERR)
		err_exit(0, "move failed: %d, %d", pos.row, pos.col);

	return 1;
}

/* tw->win is assumed to be the correct size 
 * when this function is called, so this simply
 * syncronizes the aug_term structure to the
 * right size according to tw->win 
 */
void term_win_resize(struct aug_term_win *tw) {
	int rows, cols;
	
	/* get the size of the window */
	win_dims(tw->win, &rows, &cols);

	fprintf(stderr, "term_win: resize to %d, %d\n", rows, cols);
	if(tw->term != NULL)
		if(term_resize(tw->term, rows, cols) != 0)
			err_exit(errno, "error resizing terminal!");
}