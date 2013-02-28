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