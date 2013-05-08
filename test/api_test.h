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
#ifndef AUG_API_TEST_H
#define AUG_API_TEST_H

#define AUG_TEST
#include "aug.c"

#include "ncurses_test.h"
#include <ccan/tap/tap.h>
#include <ccan/array_size/array_size.h>
#include <unistd.h>

#ifdef initscr
#	undef initscr
#endif
#define initscr() ncurses_test_init(NULL)

#ifdef endwin
#	undef endwin
#endif
#define endwin() ncurses_test_end()


#ifdef raw
#	undef raw
#endif
#define raw() OK

#include "screen.c"

#undef initscr
#undef endwin

#undef raw

#include "api_test_vars.h"

#ifndef API_TEST_IO_PAUSE 
#	define API_TEST_IO_PAUSE 1
#endif

static int api_test_main(FILE *output, int argc, char *argv[]) {
	int i;
	(void)(ncurses_test_init);
	(void)(ncurses_test_end);

	ncurses_test_init_pipe();

	if(fork() == 0) {
		tap_set_output_file(output);
		tap_set_err_output_file(output);
		diag("child(%d): start", getpid());
		sleep(API_TEST_IO_PAUSE);
		kill(getppid(), SIGWINCH);
		nct_printf(api_test_user_input);
		nct_printf("\x01\x12");
		sleep(8);
		sleep(API_TEST_IO_PAUSE); //usleep(100000);
		nct_printf(api_test_on_r_response);
		sleep(API_TEST_IO_PAUSE);

		nct_printf("echo 'blah'\r");
		nct_printf("echo 'asdfasdfasdf'\r");
		nct_printf("echo 'wertwertwert'\r");
		for(i = 0; i < 5; i++) {
			nct_printf("echo valgrind is sloooow...\r");
			sleep(1);
		}

		nct_printf("exit\r");
		diag("child: end");
		exit(0);
	}
	else {
		/* api_test + unload */
		tap_set_output_file(output);
		tap_set_err_output_file(output);
		plan_tests(131 + 4); 
		diag("test the plugin api");
		diag("parent: start");
		fflush(output);
		aug_main(argc, argv);
		diag("parent: end");
		return 0;
	}
}

#endif /* AUG_API_TEST_H */