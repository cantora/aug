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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ccan/tap/tap.h>

#include "util.h"
#include "timer.h"

struct aug_test {
	void (*fn)();
	int amt;
};

void test1() {
	struct aug_timer tmr1;
	struct timeval elapsed;

	diag("++++test1++++");	
	diag("test basic functionality/sanity");
	
	ok1(timer_init(&tmr1) == 0);
	diag("sleep 50000");
	usleep(50000);
	
	ok1(timer_elapsed(&tmr1, &elapsed) == 0);

	ok1(timer_thresh(&tmr1, 0, 50000) == 1);
	ok1(timer_thresh(&tmr1, 0, 51000) == 0);
	ok1(timer_thresh(&tmr1, 0, 55000) == 0);
	ok1(timer_thresh(&tmr1, 0, 60000) == 0);
	ok1(timer_thresh(&tmr1, 0, 75000) == 0);
	ok1(timer_thresh(&tmr1, 0, 100000) == 0);

	diag("elapsed: %d,%d", (int) elapsed.tv_sec, (int) elapsed.tv_usec);
	ok1( elapsed.tv_sec == 0 );
	ok1( elapsed.tv_usec > 49999 );
	ok1( elapsed.tv_usec < 50500 );

#define TEST1AMT 1 + 1 + 6 + 3
	diag("----test1----\n#");

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

