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