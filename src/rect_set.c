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

#include "rect_set.h"

#include <stdlib.h>

static inline size_t rect_set_size(const struct aug_rect_set *rs) {
	return rs->cols*rs->rows;
}

static inline size_t rect_set_index(const struct aug_rect_set *rs, size_t col, size_t row) {
	return row*rs->cols + col;
}

void rect_set_on(struct aug_rect_set *rs, size_t col, size_t row) {
	if(rs->map != NULL)
		rs->map[rect_set_index(rs, col, row)] = 1;
}

void rect_set_off(struct aug_rect_set *rs, size_t col, size_t row) {
	if(rs->map != NULL)
		rs->map[rect_set_index(rs, col, row)] = 0;
}

int rect_set_is_on(const struct aug_rect_set *rs, size_t col, size_t row) {
	if(rs->map != NULL)
		return rs->map[rect_set_index(rs, col, row)] != 0;
	else
		return 0;
}

int rect_set_init(struct aug_rect_set *rs, size_t cols, size_t rows) {
	rs->cols = cols;
	rs->rows = rows;

	if(rect_set_size(rs) > 0) {
		rs->map = malloc( rect_set_size(rs) * sizeof(uint8_t) );
		if(rs->map == NULL)
			return -1;
	}
	else
		rs->map = NULL;

	rect_set_clear(rs);

	return 0;
}

void rect_set_free(struct aug_rect_set *rs) {
	if(rs->map != NULL) {
		free(rs->map);
		rs->map = NULL;
	}
}

void rect_set_clear(struct aug_rect_set *rs) {
	size_t i, sz;
	sz = rect_set_size(rs);
	for(i = 0; i < sz; i++)
		rs->map[i] = 0;
}

void rect_set_add(struct aug_rect_set *rs, size_t col_start, size_t row_start,
		size_t col_end, size_t row_end) {
	size_t row, col;

	for(row = row_start; row < row_end && row < rs->rows; row++)
		for(col = col_start; col < col_end && col < rs->cols; col++)
			rect_set_on(rs, col, row);
}

static void cut_out_rect(struct aug_rect_set *rs, size_t col, size_t row, 
		struct aug_rect_set_rect *rect) {
	size_t col_width, row_width, tmp_col_width;
	
	rect->col_start = col;
	rect->row_start = row;

	rect_set_off(rs, col, row);
	col_width = 1;
	row_width = 1;

	/* find the column width of this rect that we are cutting out */
	for(col += 1; col < rs->cols; col++) {
		if(!rect_set_is_on(rs, col, row))
			break;

		rect_set_off(rs, col, row);
		col_width++;
	}

	/* find the row width of this rect that we are cutting out */
	for(row += 1; row < rs->rows; row++) {
		/* check if this row has a different width or is shifted. if so
		 * then it would be a start of a differect rect, so break. */
		if(rect->col_start > 0 && rect_set_is_on(rs, rect->col_start-1, row))
			break;

		/* check if this row has the same width as our rect */
		tmp_col_width = 0;
		for(col = rect->col_start; col < rs->cols; col++) {
			if(!rect_set_is_on(rs, col, row))
				break;

			if(tmp_col_width > col_width)
				goto done;

			tmp_col_width++;
		}

		if(tmp_col_width != col_width) 
			goto done;

		for(col = rect->col_start; col < rect->col_start + col_width; col++)
			rect_set_off(rs, col, row);
		row_width++;
	}

done:
	rect->col_end = rect->col_start + col_width;
	rect->row_end = rect->row_start + row_width;
}

int rect_set_pop(struct aug_rect_set *rs, struct aug_rect_set_rect *rect) {
	size_t row, col;

	for(row = 0; row < rs->rows; row++)
		for(col = 0; col < rs->cols; col++)
			if(rect_set_is_on(rs, col, row)) {
				cut_out_rect(rs, col, row, rect);
				return 0;
			}
	
	return -1;
}
