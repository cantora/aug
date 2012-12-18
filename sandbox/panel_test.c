#include <panel.h>
#include <string.h>

#define NLINES 10
#define NCOLS 40

void init_wins(WINDOW **wins, int n);
void win_show(WINDOW *win, char *label, int label_color);
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);

void fill_win(WINDOW *win, int ch) {
	int lines, cols, i, j;

	getmaxyx(win, lines, cols);

	for(i = 0; i < lines; i++)
		for(j = 0; j < cols; j++)
			mvwaddch(win, i, j, ch);
		
}

int main() {
	WINDOW *my_wins[3];
	WINDOW *dwin;
	WINDOW *upper, *lower;
	PANEL  *my_panels[3];
	PANEL  *top;
	int ch;
	int i, j;

	/* Initialize curses */
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	/* Initialize all the colors */
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_BLUE, COLOR_BLACK);
	init_pair(4, COLOR_CYAN, COLOR_BLACK);

	for(i = 0; i < LINES; i++)
		for(j = 0; j < COLS; j++)
			mvaddch(i, j, '+');
	doupdate();
	
	//base = newwin(LINES-2, COLS-2, 1, 1); //derwin(stdscr, LINES-2, COLS-2, 1, 1);
	//base_panel = new_panel(base);

	upper = derwin(stdscr, 8, COLS, 0, 0);
	lower = derwin(stdscr, LINES-8, COLS, 8, 0);
	init_wins(my_wins, 3);

	/* Attach a panel to each window */ 	/* Order is bottom up */
	my_panels[0] = new_panel(my_wins[0]); 	/* Push 0, order: stdscr-0 */
	my_panels[1] = new_panel(my_wins[1]); 	/* Push 1, order: stdscr-0-1 */
	my_panels[2] = new_panel(my_wins[2]); 	/* Push 2, order: stdscr-0-1-2 */

	/* Set up the user pointers to the next panel */
	set_panel_userptr(my_panels[0], my_panels[1]);
	set_panel_userptr(my_panels[1], my_panels[2]);
	set_panel_userptr(my_panels[2], my_panels[0]);

	dwin = derwin(my_wins[1], NLINES-4, NCOLS-10, 3, 3);
	
	/* Update the stacking order. 2nd panel will be on top */
	update_panels();

	/* Show it on the screen */
	attron(COLOR_PAIR(4));
	mvprintw(LINES - 2, 0, "Use tab to browse through the windows (F1 to Exit)");
	attroff(COLOR_PAIR(4));
	doupdate();

	top = my_panels[2];
	
	i = 0;
	while((ch = getch()) != 'q')
	{	switch(ch)
		{	case 9:
				top = (PANEL *)panel_userptr(top);
				top_panel(top);
				break;
		}
		//bottom_panel(base_panel);
		fill_win(dwin, 0x62 + (i % 13));
		fill_win(upper, 0x41 + (i % 16));
		fill_win(lower, 0x51 + (i % 9));
		i++;
		touchwin(stdscr);
		update_panels();
		doupdate();
	}
	endwin();
	return 0;
}

/* Put all the windows */
void init_wins(WINDOW **wins, int n)
{	int x, y, i;
	char label[80];

	y = 2;
	x = 10;
	for(i = 0; i < n; ++i)
	{	wins[i] = newwin(NLINES, NCOLS, y, x);
		sprintf(label, "Window Number %d", i + 1);
		win_show(wins[i], label, i + 1);
		y += 3;
		x += 7;
	}
}

/* Show the window with a border and a label */
void win_show(WINDOW *win, char *label, int label_color)
{	
	int height, width;
	(void)(height);

	getmaxyx(win, height, width);

	box(win, 0, 0);
	mvwaddch(win, 2, 0, ACS_LTEE); 
	mvwhline(win, 2, 1, ACS_HLINE, width - 2); 
	mvwaddch(win, 2, width - 1, ACS_RTEE); 
	
	print_in_middle(win, 1, 0, width, label, COLOR_PAIR(label_color));
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)
{	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}
