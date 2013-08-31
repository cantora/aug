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
#include <time.h>

#include "util.h"
#include "rect_set.h"

struct aug_test {
	void (*fn)();
	int amt;
};

void print_rect(struct aug_rect_set_rect *rect) {
	printf("%zd->%zd, %zd->%zd\n", rect->col_start, rect->col_end, rect->row_start, rect->row_end);
}

void test1() {
	struct aug_rect_set rs;
	size_t i, j, k;
	int one_on;
	struct aug_rect_set_rect badrects[] = {
		{341, 341, 10, 34},
		{834, 835, 0, 1},
		{10, 9, 0, 1},
		{74, 81, 43, 43},
		{0, 10, 66, 70},
		{0, 10, 45, 31}
	};

	diag("++++test1++++");	
	diag("test basic functionality/sanity");

	ok1(rect_set_init(&rs, 834, 65) == 0);

	one_on = 0;
	for(i = 0; i < 65; i++) {
		for(j = 0; j < 834;	j++) 
			if(rect_set_is_on(&rs, j, i)) {
				one_on = 1;
				break;
			}
		if(one_on != 0)
			break;
	}
	ok1(one_on == 0);

	ok1(rect_set_is_on(&rs, 734, 39) == 0);

	diag("set on");
	rect_set_on(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) != 0);

	diag("set off");
	rect_set_off(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);	

	diag("set on");
	rect_set_on(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) != 0);

	diag("clear");
	rect_set_clear(&rs);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);	
		
	diag("test bad rects");
	one_on = 0;
	for(k = 0; k < ARRAY_SIZE(badrects); k++)
		rect_set_add(&rs, 
			badrects[k].col_start, badrects[k].row_start,
			badrects[k].col_end, badrects[k].row_end
		);

	for(i = 0; i < 65; i++) {
		for(j = 0; j < 834;	j++) 
			if(rect_set_is_on(&rs, j, i)) {
				one_on = 1;
				break;
			}
		if(one_on != 0)
			break;
	}
	ok1(one_on == 0);

#define TEST1AMT 8
	diag("----test1----\n#");
	rect_set_free(&rs);
}

void test2() {
	struct aug_rect_set rs;
	struct aug_rect_set_rect input_rects[] = {
		{	0,	1,	0,	1},
		{	2,	5,	0,	1},
		{	2,	5,	1,	6},
		{	74,	75,	0,	21},
		{	2,	4,	6,	21},
		{	0,	75, 15,	17}
	};
	struct aug_rect_set_rect output_rects[] = {
		{	0,	1,	0,	1},
		{	2,	5,	0,	6},
		{	74,	75,	0,	15},
		{	2,	4,	6,	15},
		{	0,	75, 15,	17},
		{	2,	4,	17,	21},
		{	74,	75,	17,	21}
	};
	struct aug_rect_set_rect rect;
	size_t i;
	int amt_correct, rects_identical;
	

	diag("++++test2++++");	
	diag("test add/pop (1)");

	ok1(rect_set_init(&rs, 75, 21) == 0);

	for(i = 0; i < ARRAY_SIZE(input_rects); i++)
		rect_set_add(&rs, 
			input_rects[i].col_start, input_rects[i].row_start,
			input_rects[i].col_end, input_rects[i].row_end
		);

	amt_correct = 1;
	rects_identical = 1;
	for(i = 0; rect_set_pop(&rs, &rect) != 0; i++) {
		if(i >= ARRAY_SIZE(output_rects)) {
			amt_correct = 0;
			break;
		}

		if(memcmp(&rect, &output_rects[i], sizeof(rect)) != 0)
			rects_identical = 0;
	}

	ok1(amt_correct = 1);
	ok1(rects_identical = 1);
#define TEST2AMT 3
	diag("----test2----\n#");
	rect_set_free(&rs);
}

void test3() {
	struct aug_rect_set rs;
	struct aug_rect_set_rect input_rects[] = {
		{	34,	37,	1,	20},
		{	27,	41,	5,	8},
		{	56,	72,	11,	16},
		{	66,	69,	14,	18},
		{	68,	74, 17,	20}
	};
	struct aug_rect_set_rect output_rects[] = {
		{	34,	37,	1,	5},
		{	27,	41,	5,	8},
		{	34, 37, 8,	20},
		{	56,	72,	11,	16},
		{	66,	69,	16,	17},
		{	66,	68,	17,	18},
		{	68,	74,	17,	20}
	};
	struct aug_rect_set_rect rect;
	size_t i;
	int amt_correct, rects_identical;
	

	diag("++++test3++++");	
	diag("test add/pop (1)");

	ok1(rect_set_init(&rs, 75, 21) == 0);

	for(i = 0; i < ARRAY_SIZE(input_rects); i++)
		rect_set_add(&rs, 
			input_rects[i].col_start, input_rects[i].row_start,
			input_rects[i].col_end, input_rects[i].row_end
		);

	amt_correct = 1;
	rects_identical = 1;
	for(i = 0; rect_set_pop(&rs, &rect) != 0; i++) {
		if(i >= ARRAY_SIZE(output_rects)) {
			amt_correct = 0;
			break;
		}

		if(memcmp(&rect, &output_rects[i], sizeof(rect)) != 0)
			rects_identical = 0;
	}

	ok1(amt_correct = 1);
	ok1(rects_identical = 1);
#define TEST3AMT 3
	diag("----test3----\n#");
	rect_set_free(&rs);
}

void test4() {
	struct aug_rect_set rs;
	size_t i, j, k;
	int one_on;
	struct aug_rect_set_rect badrects[] = {
		{341, 341, 10, 34},
		{834, 835, 0, 1},
		{10, 9, 0, 1},
		{74, 81, 43, 43},
		{0, 10, 66, 70},
		{0, 10, 45, 31}
	};
	struct aug_rect_set_rect rect;

	diag("++++test4++++");	
	diag("test basic rect_set of dims 0x0");

	ok1(rect_set_init(&rs, 0, 0) == 0);

	one_on = 0;
	for(i = 0; i < 65; i++) {
		for(j = 0; j < 834;	j++) 
			if(rect_set_is_on(&rs, j, i)) {
				one_on = 1;
				break;
			}
		if(one_on != 0)
			break;
	}
	ok1(one_on == 0);

	ok1(rect_set_is_on(&rs, 734, 39) == 0);

	diag("set on");
	rect_set_on(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);

	diag("set off");
	rect_set_off(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);	

	diag("set on");
	rect_set_on(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);

	diag("clear");
	rect_set_clear(&rs);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);	
		
	diag("test bad rects");
	one_on = 0;
	for(k = 0; k < ARRAY_SIZE(badrects); k++)
		rect_set_add(&rs, 
			badrects[k].col_start, badrects[k].row_start,
			badrects[k].col_end, badrects[k].row_end
		);

	for(i = 0; i < 65; i++) {
		for(j = 0; j < 834;	j++) 
			if(rect_set_is_on(&rs, j, i)) {
				one_on = 1;
				break;
			}
		if(one_on != 0)
			break;
	}
	ok1(one_on == 0);

	ok1(rect_set_pop(&rs, &rect) != 0);

#define TEST4AMT 9
	diag("----test4----\n#");
	rect_set_free(&rs);
}

void test5() {
	struct aug_rect_set rs;
	size_t i, j, k;
	int one_on;
	struct aug_rect_set_rect badrects[] = {
		{341, 341, 10, 34},
		{834, 835, 0, 1},
		{10, 9, 0, 1},
		{74, 81, 43, 43},
		{0, 10, 66, 70},
		{0, 10, 45, 31}
	};
	struct aug_rect_set_rect rect;

	diag("++++test5++++");	
	diag("test basic rect_set of dims 0x80");

	ok1(rect_set_init(&rs, 0, 80) == 0);

	one_on = 0;
	for(i = 0; i < 65; i++) {
		for(j = 0; j < 834;	j++) 
			if(rect_set_is_on(&rs, j, i)) {
				one_on = 1;
				break;
			}
		if(one_on != 0)
			break;
	}
	ok1(one_on == 0);

	ok1(rect_set_is_on(&rs, 734, 39) == 0);

	diag("set on");
	rect_set_on(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);

	diag("set off");
	rect_set_off(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);	

	diag("set on");
	rect_set_on(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);

	diag("clear");
	rect_set_clear(&rs);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);	
		
	diag("test bad rects");
	one_on = 0;
	for(k = 0; k < ARRAY_SIZE(badrects); k++)
		rect_set_add(&rs, 
			badrects[k].col_start, badrects[k].row_start,
			badrects[k].col_end, badrects[k].row_end
		);

	for(i = 0; i < 65; i++) {
		for(j = 0; j < 834;	j++) 
			if(rect_set_is_on(&rs, j, i)) {
				one_on = 1;
				break;
			}
		if(one_on != 0)
			break;
	}
	ok1(one_on == 0);

	ok1(rect_set_pop(&rs, &rect) != 0);

#define TEST5AMT 9
	diag("----test5----\n#");
	rect_set_free(&rs);
}

void test6() {
	struct aug_rect_set rs;
	size_t i, j, k;
	int one_on;
	struct aug_rect_set_rect badrects[] = {
		{341, 341, 10, 34},
		{834, 835, 0, 1},
		{10, 9, 0, 1},
		{74, 81, 43, 43},
		{0, 10, 66, 70},
		{0, 10, 45, 31}
	};
	struct aug_rect_set_rect rect;

	diag("++++test6++++");	
	diag("test basic rect_set of dims 73x0");

	ok1(rect_set_init(&rs, 73, 0) == 0);

	one_on = 0;
	for(i = 0; i < 65; i++) {
		for(j = 0; j < 834;	j++) 
			if(rect_set_is_on(&rs, j, i)) {
				one_on = 1;
				break;
			}
		if(one_on != 0)
			break;
	}
	ok1(one_on == 0);

	ok1(rect_set_is_on(&rs, 734, 39) == 0);

	diag("set on");
	rect_set_on(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);

	diag("set off");
	rect_set_off(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);	

	diag("set on");
	rect_set_on(&rs, 734, 39);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);

	diag("clear");
	rect_set_clear(&rs);
	ok1(rect_set_is_on(&rs, 734, 39) == 0);	
		
	diag("test bad rects");
	one_on = 0;
	for(k = 0; k < ARRAY_SIZE(badrects); k++)
		rect_set_add(&rs, 
			badrects[k].col_start, badrects[k].row_start,
			badrects[k].col_end, badrects[k].row_end
		);

	for(i = 0; i < 65; i++) {
		for(j = 0; j < 834;	j++) 
			if(rect_set_is_on(&rs, j, i)) {
				one_on = 1;
				break;
			}
		if(one_on != 0)
			break;
	}
	ok1(one_on == 0);

	ok1(rect_set_pop(&rs, &rect) != 0);

#define TEST6AMT 9
	diag("----test6----\n#");
	rect_set_free(&rs);
}

void test7() {
	struct aug_rect_set rs;
	struct aug_rect_set_rect r;
	int amt;

	diag("++++test7++++");	
	ok1(rect_set_init(&rs, 143, 47) == 0);

	rect_set_add(&rs, 110, 0, 142, 15);

	amt = 0;
	while(rect_set_pop(&rs, &r) == 0) {
		amt++;
		diag("got rect: %d->%d, %d->%d", 
				r.col_start, r.col_end,
				r.row_start, r.row_end);
	}

	ok1(amt > 0);
	ok1(amt == 1);

	ok1(r.col_start == 110);
	ok1(r.col_end == 142);
	ok1(r.row_start == 0);
	ok1(r.row_end == 15);

#define TEST7AMT 1 + 2 + 4
	diag("----test7----\n#");
	rect_set_free(&rs);
}

int main()
{
	int i, len, total_tests;
#define TESTN(_num) {test##_num, TEST##_num##AMT}
	struct aug_test tests[] = {
		TESTN(1),
		TESTN(2),
		TESTN(3),
		TESTN(4),
		TESTN(5),
		TESTN(6),
		TESTN(7)
	};

	total_tests = 0;
	len = AUG_ARRAY_SIZE(tests);
	for(i = 0; i < len; i++) {
		total_tests += tests[i].amt;
	}

	plan_tests(total_tests);

	srandom(time(NULL));

	for(i = 0; i < len; i++) {
		(*tests[i].fn)();
	}

	return exit_status();
}

