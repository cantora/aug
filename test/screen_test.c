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

#include "screen.h"

struct aug_test {
	void (*fn)();
	int amt;
};

void test1() {
	uint32_t ch;
	char s[8];
	diag("++++test1++++");	

	for(ch = 0; ch <= 0x7f; ch++) {
		ok1(screen_unctrl(ch, s) == 0);
		diag("0x%02x: %s", ch, s);
	}

#define TEST1AMT 0x7f+1
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

