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
#ifndef AUG_ATTR_H
#define AUG_ATTR_H

#include "vterm.h"

#include "ncurses.h"

#define AUG_ANSI_COLORS 8
#define AUG_REQ_COLORS (AUG_ANSI_COLORS + 1) /* 8 ansi + default */
#define AUG_REQ_PAIRS AUG_REQ_COLORS*AUG_REQ_COLORS 
#define AUG_TOTAL_ANSI_COLORS (AUG_ANSI_COLORS*2) /* normal + bright */

const VTermColor VTERM_DEFAULT_COLOR; /* placeholder for default color */

#ifndef COLOR_DEFAULT
#	define COLOR_DEFAULT -1
#endif

void attr_curses_colors_to_curses_pair(int fg, int bg, int *pair);
void attr_vterm_index_to_curses_index(int vterm_index, int *curses_index, int *bright);
int attr_vterm_color_to_curses_color(VTermColor color, int *curses_color, int *bright);
void attr_vterm_color_to_nearest_curses_color(VTermColor color, int *curses_color, int *bright);
void attr_vterm_pair_to_curses_pair(VTermColor fg, VTermColor bg, attr_t *attr, int *pair);
void attr_vterm_attr_to_curses_attr(const VTermScreenCell *cell, attr_t *attr);

#endif /* AUG_ATTR_H */