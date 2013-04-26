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
#ifndef AUG_NCURSES_H
#define AUG_NCURSES_H

#ifndef _XOPEN_SOURCE
#	define _XOPEN_SOURCE 700
#endif

#ifndef _XOPEN_SOURCE_EXTENDED
#	define _XOPEN_SOURCE_EXTENDED 1
#endif

#if defined(__APPLE__)
#	include <ncurses.h>
#else
#	include <ncursesw/curses.h>
#endif

#endif /* AUG_NCURSES_H */