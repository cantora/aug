#include <ncurses.h>


int main() {
	int ch;

	initscr();
	keypad(stdscr, true);

	while( (ch = getch() ) != 'q' ) {
		clear();
		mvprintw(0, 0, "0x%x", ch);
		move(1,0);
		refresh();
	}

	endwin();

	return 0;
}