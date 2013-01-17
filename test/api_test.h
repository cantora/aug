#ifndef AUG_API_TEST_H
#define AUG_API_TEST_H

#define AUG_TEST
#include "aug.c"

#include "ncurses_test.h"
#ifndef NCT_USE_SCREEN
#	include <ccan/tap/tap.h>
#else
#	include "stderr_tap.h"
#endif
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

static int api_test_main(int argc, char *argv[]) {
	(void)(ncurses_test_init);
	(void)(ncurses_test_end);

	ncurses_test_init_pipe();

	if(fork() == 0) {
		diag("child: start");
		sleep(API_TEST_IO_PAUSE);
		kill(getppid(), SIGWINCH);
		nct_printf(api_test_user_input);
		nct_printf("\x01\x12");
		sleep(API_TEST_IO_PAUSE); //usleep(100000);
		nct_printf(api_test_on_r_response);
		sleep(API_TEST_IO_PAUSE);
		sleep(8);
		nct_printf("\nexit\n");
		/*diag("child: end");*/
		exit(0);
	}
	else {
		//plan_no_plan();
		diag("test the plugin api");
		diag("parent: start");
		aug_main(argc, argv);
		diag("parent: end");
		return 0;
	}
}

#endif /* AUG_API_TEST_H */