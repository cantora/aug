#include <ncurses.h>

int main() {
	int ch;

	initscr();
	if(raw() == ERR) 
		goto fail;
	if(noecho() == ERR) 
		goto fail;
	if(nodelay(stdscr, true) == ERR) 
		goto fail;
	if(keypad(stdscr, true) == ERR)
		goto fail;
	if(nonl() == ERR) 
		goto fail;
	if(meta(stdscr, true) == ERR)
		goto fail;

	while( (ch = getch() ) != 'q' ) {
		if(ch == ERR) {
			usleep(100000);
			continue;
		}

		clear();
		mvprintw(0, 0, "0x%x", ch);
		move(1,0);
		refresh();
	}

	endwin();

	return 0;
fail:
	endwin();
	printf("error\n");
	return 1;
}