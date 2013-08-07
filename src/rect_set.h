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

#ifndef AUG_RECT_SET_H
#define AUG_RECT_SET_H

#include <stdint.h>
#include <string.h>

struct aug_rect_set {
	uint8_t *map;
	size_t cols;
	size_t rows;
};

struct aug_rect_set_rect {
	size_t col_start;
	size_t col_end;
	size_t row_start;
	size_t row_end;
};

int rect_set_init(struct aug_rect_set *rs, size_t cols, size_t rows);
void rect_set_free(struct aug_rect_set *rs);

/* set point at @col, @row on */
void rect_set_on(struct aug_rect_set *rs, size_t col, size_t row);

/* set point at @col, @row off */
void rect_set_off(struct aug_rect_set *rs, size_t col, size_t row);

/* return non-zero if point described by @col, @row is on */
int rect_set_is_on(const struct aug_rect_set *rs, size_t col, size_t row);

/* turn off all points */
void rect_set_clear(struct aug_rect_set *rs);

/* turn on points within the rectangle described by the vertex args */
void rect_set_add(struct aug_rect_set *rs, size_t col_start, size_t row_start,
		size_t col_end, size_t row_end);

/* searches for any "on" points in the map and expands that point into 
 * a rectangle, deletes that rectangle from the map and returns. if no
 * "on" points are found, returns non-zero. 
 *
 * a note about efficiency: the location
 * of the last deleted rectangle is not kept, so some effort is wasted checking
 * places which we may know are off. maintaining the location of the last deleted
 * rectangle could improve the performance of rect_set_pop but it would complicate
 * things. */
int rect_set_pop(struct aug_rect_set *rs, struct aug_rect_set_rect *rect);


#endif /* AUG_RECT_SET_H */
