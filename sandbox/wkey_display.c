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
#include <stdint.h>

#include "ncurses.h"

int main() {
	uint32_t ch;
	const char *err = "setup error";

	initscr();
	if(raw() == ERR) 
		goto fail;
	if(noecho() == ERR) 
		goto fail;
	if(nodelay(stdscr, true) == ERR) 
		goto fail;
	/*if(keypad(stdscr, true) == ERR)
		goto fail;*/
	if(nonl() == ERR) 
		goto fail;
	/*if(meta(stdscr, true) == ERR)
		goto fail;*/

	while(1) {
		if(get_wch(&ch) == ERR) {
			usleep(100000);
			continue;
		}
			
		if(ch == 'q') 
			break;

		clear();
		mvprintw(0, 0, "0x%08x", ch);
		move(1,0);
		refresh();
	}

	endwin();

	return 0;
fail:
	endwin();
	printf("error: %s\n", err);
	return 1;
}