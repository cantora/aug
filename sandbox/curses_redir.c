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