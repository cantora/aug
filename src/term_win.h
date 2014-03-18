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
#ifndef AUG_TERM_WIN
#define AUG_TERM_WIN

#include "aug_types.h"

#include "term.h"
#include "rect_set.h"

struct aug_term_win {
	WINDOW *win;
	struct aug_term *term;
	struct aug_rect_set deferred_damage;
	struct {
		int row;
		int col;
	} cursor;
};

void term_win_init(struct aug_term_win *tw, WINDOW *win);
void term_win_free(struct aug_term_win *tw);
void term_win_reset_damage(struct aug_term_win *tw);
void term_win_defer_damage(struct aug_term_win *tw, size_t col_start,
		size_t col_end, size_t row_start, size_t row_end);
void term_win_set_term(struct aug_term_win *tw, struct aug_term *term);
void term_win_dims(const struct aug_term_win *tw, int *rows, int *cols);
int term_win_cell(const struct aug_term_win *tw, int row, int col,
					int color_on, struct aug_cell *cell);
void term_win_update_cell(struct aug_term_win *tw, int row, int col, int color_on);
void term_win_refresh(struct aug_term_win *tw, int color_on);
int term_win_damage(struct aug_term_win *tw, VTermRect rect, int color_on);
int term_win_moverect(struct aug_term_win *tw, VTermRect dest,
						VTermRect src, int color_on);
int term_win_movecursor(struct aug_term_win *tw, int pos_row, int pos_col,
							int color_on, int do_callbacks);
void term_win_resize(struct aug_term_win *tw, WINDOW *win);

#endif /* AUG_TERM_WIN */
