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
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "ncurses_test.h"
#include <ccan/tap/tap.h>

int main() {
	WINDOW *win;
	int i, j, maxy, maxx;
	char ranchars[16];
	char cmdbuf[512];
#	define FILEPATH "/tmp/ncurses_test.out"
	
	srand(time(NULL) );

	plan_tests(3);

	ncurses_test_init(FILEPATH);
	
	AUG_PTR_NON_NULL( (win = derwin(stdscr, LINES-10, COLS-10, 3, 3) ) );
	ok1(win != NULL);

	box(stdscr, 0, 0);
	wrefresh(stdscr);
	box(win, 0, 0);	
	getmaxyx(win, maxy, maxx);
	
	for(i = 1; i < maxy-1; i++)
		for(j = 1; j < maxx-1; j++) {
			mvwaddch(win, i, j, '+');
		}	

	for(i = 0; (size_t) i < sizeof(ranchars)-1; i++) 
		ranchars[i] = (rand() % 26) + 0x41;
	ranchars[sizeof(ranchars)-1] = '\0';

	mvwprintw(win, 5, 5, ranchars, maxy, maxx);
	wrefresh(win);

	snprintf(cmdbuf, 512, "grep	-F '%s' " FILEPATH " 1>/dev/null 2>&1", ranchars);
	ok1( system(cmdbuf) == 0 );

	nct_printf("\n");
	/*nct_flush();*/

	ok1(wgetch(win) == '\n');	
	
	ncurses_test_end();

	unlink(FILEPATH);

	return 0;
}