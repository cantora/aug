#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#include "attr.h"

int main() {
	int i,k,offset,x,atr;
	int pair,fg,bg;
	const char *err = NULL;	
	const char *fmt = "%02d: ####";
	int colors[] = { COLOR_DEFAULT, COLOR_BLACK, COLOR_RED, COLOR_GREEN,
					COLOR_YELLOW, COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE };
	int arrlen = sizeof(colors)/sizeof(int);

	initscr();

	if(!has_colors()) {
		err = "terminal doesnt support color";
		goto cleanup;
	}

	if(start_color() == ERR) {
		err = "failure to start curses color";
		goto cleanup;
	}

	use_default_colors();

	for(i = 0; i < arrlen; i++) {
		fg = colors[i];
		for(k = 0; k < arrlen; k++) {
			bg = colors[k];
			//pair = i*9 + k;
			attr_curses_colors_to_curses_pair(fg, bg, &pair);
			if(pair == 0) { /* pair 0 is default on default */
				continue;
			}
			
			init_pair(pair, fg, bg);
		}
	}

	x = 0;
	for(offset = 0; offset < 9*9; offset += 9) {
		atr = A_NORMAL;	
		for(i = 0; i < 9; i++) {
			attr_set(atr, offset+i, NULL);
			mvprintw(i, x, fmt, offset+i);
		}
	
		atr = A_BOLD;
		for(i = 0; i < 9; i++) {
			attr_set(atr, offset+i, NULL);
			mvprintw(9+i, x, fmt, offset+i);
		}
	
		x += 12;
	}

	refresh();
	getch();
/*
	for(i = 0; i < 9; i++) {
		for(k = 0; k < 9; k++) {
			pair = i*9 + k;
			if(pair == 0) {
				continue;
			}
			
			fg = (8-i > 7)? -1 : 8-i;
			bg = (8-k > 7)? -1 : 8-k;

			init_pair(pair, 1, 3);
		}
	}

	refresh();
	getch();
*/
cleanup:
	endwin();

	if( err != NULL )
		printf("error: %s\n", err);
	
	return 0;
}