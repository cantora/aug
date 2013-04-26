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
#include <ncurses.h>


int main() {
	WINDOW *win;
	int i, j, maxy, maxx;

	initscr();

	win = newwin(10, 100, 5, 25);
	box(win, 0, 0);
	getmaxyx(win, maxy, maxx);
	
	for(i = 1; i < maxy-1; i++)
		for(j = 1; j < maxx-1; j++) {
			mvwaddch(win, i, j, '+');		
	}	

	wrefresh(win);
	wgetch(win);
	endwin();

	return 0;
}