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
#include "attr.h"
#include <assert.h>
#include "vterm_util.h"
#include "vterm_ansi_colors.h"

const VTermColor VTERM_DEFAULT_COLOR = {
	.red = 1,
	.green = 1,
	.blue = 1
};

/* this relies on the ncurses implementation setting the
 * COLOR_* variables to 0 through 7 with COLOR_DEFAULT = -1.
 * it creates a bijection from {-1, 0, ..., 7} to 
 * {0, ..., 8} such that -1 maps to 0. this is all
 * done because the ncurses pair 0 is always 
 * default foreground on default background, so 
 * COLOR_DEFAULT, COLOR_DEFAULT = -1, -1, must map
 * to pair 0.
 */
static int map_curses_color_index(int index) {
	int result;

	assert(index <= 7);
	assert(index >= -1);

	switch(index) {
	case -1:
		result = 0;
	default:
		result = index + 1;		
	}		

	return result;
}

/* assumes that curses COLOR_* variables are in the range 0-7
 */
void attr_curses_colors_to_curses_pair(int fg, int bg, int *pair) {
	*pair = map_curses_color_index(fg)*AUG_REQ_COLORS + map_curses_color_index(bg);

	assert(*pair < AUG_REQ_PAIRS);
	assert(*pair >= 0);
}

/* *vterm_index* is a index into the table of 
 * libvterm ansi color definitions. *curses_index*
 * is an output parameter that will hold the 
 * equivalent ncurses color number. *bright* is
 * an output parameter which will be non-zero if the 
 * color is bright and 0 if it is normal.
 */
void attr_vterm_index_to_curses_index(int vterm_index, int *curses_index, int *bright) {
	assert(vterm_index >= -1 && vterm_index < 16);
	
	if(vterm_index > 7) {
		vterm_index = vterm_index%8;
		*bright = 1;
	}
	else
		*bright = 0;

	switch(vterm_index) {
	case -1:
		*curses_index = COLOR_DEFAULT;
		break;
	case VTERM_ANSI_BLACK:
		*curses_index = COLOR_BLACK;
		break;
	case VTERM_ANSI_RED:
		*curses_index = COLOR_RED;
		break;
	case VTERM_ANSI_GREEN:
		*curses_index = COLOR_GREEN;
		break;
	case VTERM_ANSI_YELLOW:
		*curses_index = COLOR_YELLOW;
		break;
	case VTERM_ANSI_BLUE:
		*curses_index = COLOR_BLUE;
		break;
	case VTERM_ANSI_MAGENTA:
		*curses_index = COLOR_MAGENTA;
		break;
	case VTERM_ANSI_CYAN:
		*curses_index = COLOR_CYAN;
		break;
	default: /* VTERM_ANSI_WHITE */
		*curses_index = COLOR_WHITE;
	}
}

int attr_vterm_color_to_curses_color(VTermColor color, int *curses_color, int *bright) {
	int i;

	for(i = 0; i < AUG_TOTAL_ANSI_COLORS; i++) {
		if( vterm_color_equal(&color, &vterm_ansi_colors[i]) )
			break;
	}
	if(i == AUG_TOTAL_ANSI_COLORS) {
		if( vterm_color_equal(&color, &VTERM_DEFAULT_COLOR) ) {
			i = -1;
		}
		else
			goto fail;
	}
	
	attr_vterm_index_to_curses_index(i, curses_color, bright);

	return 0;
fail:
	return -1;	
}

/* smallest dist^2 determines nearest
 * ansi color
 */
void attr_vterm_color_to_nearest_curses_color(VTermColor color, int *curses_color, int *bright) {
	int i, min_index, min_dist_sq, dist_sq;

	min_dist_sq = vterm_color_dist_sq(&color, &vterm_ansi_colors[0]);
	min_index = 0;
	for(i = 1; i < AUG_TOTAL_ANSI_COLORS; i++) {
		dist_sq = vterm_color_dist_sq(&color, &vterm_ansi_colors[i]);
		if(dist_sq <= min_dist_sq) {
			min_dist_sq = dist_sq;
			min_index = i;
		}
	}

	attr_vterm_index_to_curses_index(min_index, curses_color, bright);
}

void attr_vterm_pair_to_curses_pair(VTermColor fg, VTermColor bg, attr_t *attr, int *pair) {
	int curs_fg, curs_bg, bright_fg, bright_bg;
	
	if(attr_vterm_color_to_curses_color(fg, &curs_fg, &bright_fg) != 0) {
		attr_vterm_color_to_nearest_curses_color(fg, &curs_fg, &bright_fg);
		fprintf(stderr, "attr_vterm_pair_to_curses_pair: mapped fg color %d,%d,%d to color %d\n", fg.red, fg.green, fg.blue, curs_fg);
	}
	if(attr_vterm_color_to_curses_color(bg, &curs_bg, &bright_bg) != 0) {
		attr_vterm_color_to_nearest_curses_color(bg, &curs_bg, &bright_bg);
		fprintf(stderr, "attr_vterm_pair_to_curses_pair: mapped bg color %d,%d,%d to color %d\n", bg.red, bg.green, bg.blue, curs_bg);
	}
	
	attr_curses_colors_to_curses_pair(curs_fg, curs_bg, pair);

	if(bright_fg)
		*attr |= A_BOLD;
}

/* take a libvterm cell and convert its attributes
 * to corresponding ncurses attributes. 
 */
void attr_vterm_attr_to_curses_attr(const VTermScreenCell *cell, attr_t *attr) {
	attr_t result = A_NORMAL;
	
	if(cell->attrs.bold != 0)
		result |= A_BOLD;

	if(cell->attrs.underline != 0)
		result |= A_UNDERLINE;

	if(cell->attrs.blink != 0)
		result |= A_BLINK;

	if(cell->attrs.reverse != 0)
		result |= A_REVERSE;

	*attr = result;
}

