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
#ifndef AUG_NCURSES_UTIL_H
#define AUG_NCURSES_UTIL_H

/* return zero if point (x,y) is not contained in the 
 * window else return non-zero.
 */
static inline int win_contained(const WINDOW *win, int y, int x) {
	int maxx, maxy;

	getmaxyx(win, maxy, maxx);	
	return (x < maxx) && (y < maxy);
}

static inline void win_dims(const WINDOW *win, int *rows, int *cols) {
	int maxx, maxy;

	getmaxyx(win, maxy, maxx);
	*rows = maxy;
	*cols = maxx;
	
	assert(*rows > 0);
	assert(*cols > 0);	
}

#endif /* AUG_NCURSES_UTIL_H */