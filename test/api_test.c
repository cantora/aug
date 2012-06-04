#include "api_test.h"
#include <ccan/array_size/array_size.h>
#include <ccan/tap/tap.h>

#define NTESTS 7

#define AUG_TEST
#include "aug.c"
#include "ncurses_test.h"

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

int main(int argc, char *argv[]) {
	char *args[] = {argv[0], "-c", "./test/api_test_augrc", 
						"--plugin-path", "./test/plugin/api_test:./test/plugin/fail_init:./plugin/hello", 
						"-d", "./build/log",
						NULL };
	(void)(argc);
	
	ncurses_test_init_pipe();

	if(fork() == 0) {
		diag("child: start");
		sleep(1);
		//diag("child: write to parent");
		nct_printf("\x11\x12");
		nct_printf(api_test_user_input);
		diag("child: end");
		return 0;
	}
	else {
		//plan_no_plan();
		diag("test the plugin api");
		diag("parent: start");
		aug_main(ARRAY_SIZE(args)-1, args);
		diag("parent: end");
		return exit_status();
	}
}
