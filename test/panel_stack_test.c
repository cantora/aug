#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "ncurses_test.h"
#include <ccan/tap/tap.h>

#include "panel_stack.h"

#define RETURN_IF( _expr, _status ) \
	do { \
		if( (_expr) ) { \
			return _status; \
		} \
	} while(0)

int test1() {
	PANEL *panel, *next;
	int i;
	
	diag("++++test1++++");
	diag("test for empty panel stack");
	panel_stack_top(&panel);
	ok1(panel == NULL);

	i = 0;	
	PANEL_STACK_FOREACH(panel) {
		if(i > 1)
			break;
		i++;
	}
	ok1( i == 0 );
	
	i = 0;	
	PANEL_STACK_FOREACH_SAFE(panel, next) {
		if(i > 1)
			break;
		i++;
	}
	ok1( i == 0 );
	
#define TEST1AMT 3
	diag("----test1----\n#");
	return 0;
}

int test2() {
	int dummy_plugin;
	PANEL *panel, *next;
	int i, size;
	const int *user;
	
	dummy_plugin = 1;

	diag("++++test2++++");
	diag("test pushing one panel and deleting");
	panel_stack_top(&panel);
	ok1(panel == NULL);

	panel_stack_push((struct aug_plugin *) &dummy_plugin, 10, 10, 1, 1);

	panel_stack_top(&panel);
	RETURN_IF( panel == NULL, -1);
	ok1(panel != NULL);
	
	user = panel_userptr(panel);
	RETURN_IF( user == NULL, -1);
	ok1(*user == 1);
	
	i = 0;	
	PANEL_STACK_FOREACH(panel) {
		if(i > 1)
			break;
		i++;
	}
	ok1( i == 1 );	
	
	panel_stack_size(&size);
	ok1( size == 1 );

	i = 0;
	PANEL_STACK_FOREACH_SAFE(panel, next) {
		if(i > 1)
			break;
		i++;
	}
	ok1( i == 1 );

	panel_stack_top(&panel);
	panel_stack_rm(panel);
	panel_stack_size(&size);
	ok1(size == 0);

#define TEST2AMT 7
	diag("----test2----\n#");
	return 0;
}

int test3() {
	int dummy_plugin[3];
	PANEL *panel, *next;
	int i, size;
	const int *user;

	dummy_plugin[0] = 1;
	dummy_plugin[1] = 2;
	dummy_plugin[2] = 3;
	
	diag("++++test3++++");
	diag("test pushing three panels and deleting one in a foreach loop");
	
	panel_stack_push((struct aug_plugin *) &dummy_plugin[0], 10, 10, 1, 1);
	panel_stack_push((struct aug_plugin *) &dummy_plugin[1], 10, 10, 2, 2);
	panel_stack_push((struct aug_plugin *) &dummy_plugin[2], 10, 10, 3, 3);
	
	panel_stack_size(&size);
	ok1( size == 3 );

	i = 0;
	PANEL_STACK_FOREACH(panel) {
		RETURN_IF( panel == NULL, -1);
		
		user = panel_userptr(panel);
		RETURN_IF( user == NULL, -1);
		//printf("user = %d\n", *user);
		ok1(*user == (3-i));
		i++;
	}
	
	i = 0;
	PANEL_STACK_FOREACH_SAFE(panel, next) {
		user = panel_userptr(panel);
		RETURN_IF( user == NULL, -1);

		if(*user == 2) 
			panel_stack_rm(panel);
		
		i++;
	}

	ok1(i == 3);

	panel_stack_size(&size);
	ok1(size == 2);

	panel_stack_top(&panel);
	user = panel_userptr(panel);
	ok1( *user == 3 );
	panel_stack_rm(panel);

	panel_stack_top(&panel);
	user = panel_userptr(panel);
	ok1( *user == 1 );
	panel_stack_rm(panel);

	panel_stack_size(&size);
	ok1(size == 0);
		
#define TEST3AMT 9
	diag("----test3----\n#");
	return 0;
}

int test4() {
	int **dummy;
	int i, size, amt;
	const int *user;
	PANEL *panel;
	char buf[128];

	srand(time(NULL));

	diag("++++test4++++");
	amt = (rand() % 30) + 10;

	snprintf(buf, 128, "test pushing %d panels and freeing", amt);
	diag(buf);

	dummy = malloc( sizeof(int*) * amt );
	RETURN_IF( (dummy == NULL), -1);

	for(i = 0; i < amt; i++) {
		dummy[i] = malloc( sizeof(int) );
		RETURN_IF((dummy[i] == NULL), -1);

		*(dummy[i]) = i;
		panel_stack_push((struct aug_plugin *) dummy[i], 10, 10, 0, 0);
	}
	
	panel_stack_size(&size);
	ok1( size == amt );

	i = 0;
	PANEL_STACK_FOREACH(panel) {
		user = panel_userptr(panel);
		//printf("user = %d\n", *user);
		if(*user != (amt-1-i))
			break;
		i++;
	}
	ok1(i == amt);

	panel_stack_free();

	panel_stack_size(&size);
	ok1(size == 0);
		
#define TEST4AMT 3
	diag("----test4----\n#");
	return 0;
}

struct aug_test {
	int (*fn)();
	int amt;
};

int main()
{
	int i, len, total_tests;
#	define TESTN(_num) {test##_num, TEST##_num##AMT}
#	define FILEPATH "/tmp/panel_stack_test.out"
	struct aug_test tests[] = {
		TESTN(1),
		TESTN(2),
		TESTN(3),
		TESTN(4)
	};
	(void)(nct_printf);
	(void)(nct_flush);

	len = AUG_ARRAY_SIZE(tests);
	for(i = 0; i < len; i++) {
		total_tests += tests[i].amt;
	}

	plan_tests(total_tests);

	ncurses_test_init(FILEPATH);

	for(i = 0; i < len; i++) {
		if( (*tests[i].fn)() != 0) {
			printf("# test %d failed. abort...\n", i);
			break;
		}
	}

	ncurses_test_end();
	unlink(FILEPATH);

	return exit_status();
}