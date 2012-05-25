#include "util.h"
#include <ccan/tap/tap.h>

struct aug_test {
	void (*fn)();
	int amt;
};

void test1() {
#	define TOK1 "blah"
#	define TOK2 "@#$%zlk"
#	define TOK3 "(*aksjhdn$"
#	define TOK4 "\"///\\zxkkkkkhs)(*&"
	const char *arr[] = {TOK1, TOK2, TOK3, TOK4 };
	char str[128];
	char *itr;
	int i;
#define TOKALL TOK1 ":" TOK2 ":" TOK3 ":" TOK4
	strncpy(str, TOKALL, 128);
	printf("str: %s\n", str);
	diag("test util foreach token macro");
	i = 0;
	AUG_FOREACH_TOK(itr, str, ":") {
		ok1( strcmp(itr, arr[i++]) == 0 );
	}

	diag("test whether foreach modifies the string it iterated over\n");
	ok1( strcmp(str, TOKALL) == 0 );
	printf("str: %s\n", str);

#define TEST1AMT 4
}

int main()
{
	int i, len, total_tests;
#define TESTN(_num) {test##_num, TEST##_num##AMT}
	struct aug_test tests[] = {
		TESTN(1)
	};

	len = AUG_ARRAY_SIZE(tests);
	for(i = 0; i < len; i++) {
		total_tests += tests[i].amt;
	}

	plan_tests(total_tests);

	for(i = 0; i < len; i++) {
		(*tests[i].fn)();
	}

	return exit_status();
}

