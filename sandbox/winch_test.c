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
#include <ncurses.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#if defined(__FreeBSD__)
#	include <libutil.h>
#	include <termios.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#	include <termios.h>
#	include <util.h>
#else
#	include <pty.h>
#endif

const char *logfile = "/tmp/winch_test_log";

void child_winch(int signo) {
	FILE *log = fopen(logfile, "a");
	fprintf(log, "child at pid %d got winch\n", getpid() );
	fflush(log);
	fclose(log);				
}

void parent_winch(int signo) {
	FILE *log = fopen(logfile, "a");
	fprintf(log, "parent at pid %d got winch\n", getpid() );
	fflush(log);
	fclose(log);				
}

int main() {
	int master, ch, child_id;
	struct winsize size;

	initscr();
	size.ws_row = LINES;
	size.ws_col = COLS;
	
	child_id = forkpty(&master, NULL, NULL, &size);
	
	FILE *log = fopen(logfile, "a");

	if(child_id == 0) {
		signal(SIGWINCH, child_winch);
		fprintf(log, "child at pid %d: start\n", getpid() );
		fflush(log);
		pause();
		fprintf(log, "child at pid %d: unpaused->exit\n", getpid() );
		fflush(log);
		fclose(log);
		exit(0);
	}
	else {
		signal(SIGWINCH, parent_winch);
		fprintf(log, "parent at pid %d: start\n", getpid() );
	}
		
	do {
		clear();
		mvprintw(0, 0, "dims: %d, %d\n", LINES, COLS);
		refresh();

		if(ch == 's') {
			size.ws_row--;
			size.ws_col--;
			if(ioctl(master, TIOCSWINSZ, &size) != 0) {
				fprintf(log, "ioctl error: %s\n", strerror(errno) );
				exit(1);
			}
		}
	} while( (ch = getch() ) != 'q');

	endwin();

	return 0;
}