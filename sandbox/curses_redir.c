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

#include <ncurses.h>

int main() {
	WINDOW *win;
	SCREEN *scr;
	FILE *out, *in, *to_in;
	int pd[2];
	int i, j, maxy, maxx;

	if( (out = fopen("/tmp/curses_redir.out", "w") ) == NULL) {
		fprintf(stderr, "error opening output file\n");
		exit(1);
	}

	if(pipe(pd) != 0) {
		fprintf(stderr, "pipe error: %s\n", strerror(errno) );
		exit(1);
	}

	if( (in = fdopen(pd[0], "r") ) == NULL) {
		fprintf(stderr, "error opening input file\n");
		exit(1);
	}

	if( (to_in = fdopen(pd[1], "w") ) == NULL) {
		fprintf(stderr, "error opening to_in file *\n");
		exit(1);
	}

	if( (scr = newterm(NULL, out, in) ) == NULL ) {
		fprintf(stderr, "newterm failed\n");
		exit(1);
	}

	set_term(scr);

	win = newwin(LINES, COLS, 0, 0);
	if(win == NULL) {
		fprintf(stderr, "error creating win\n");
		exit(1);
	}

	box(win, 0, 0);
	getmaxyx(win, maxy, maxx);
	
	for(i = 1; i < maxy-1; i++)
		for(j = 1; j < maxx-1; j++) {
			mvwaddch(win, i, j, '+');
	}	

	mvwprintw(win, 5, 5, "curses_redir dims: %d, %d", maxy, maxx);

	wrefresh(win);
	fprintf(to_in, "\n");
	fflush(to_in);
	wgetch(win);

	for(i = 1; i < maxy-1; i++)
		for(j = 1; j < maxx-1; j++) {
			mvwaddch(win, i, j, (j % 2 == 0)? '=' : '*');
	}	

	mvwprintw(win, 0, 0, "curses_redir dims: %d, %d", maxy, maxx);

	wrefresh(win);
	fprintf(to_in, "\n");
	fflush(to_in);
	wgetch(win);
	
	endwin();

	fclose(out);
	fclose(in);
	fclose(to_in);

	return 0;
}