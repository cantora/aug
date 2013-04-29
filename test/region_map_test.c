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
#include "region_map.h"

struct aug_test {
	void (*fn)();
	int amt;
};

void print_region(struct aug_region *reg) {
	printf("%dx%d @ (%d, %d)\n", reg->rows, reg->cols, reg->y, reg->x);
}

void test1() {
	AVL *key_regs;
	struct aug_region primary;

	diag("++++test1++++");	
	diag("test basic functionality/sanity");
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	ok1(region_map_apply(0, 10, key_regs, &primary) == -1);
	ok1(avl_count(key_regs) == 0);

	ok1(region_map_apply(1234, 0, key_regs, &primary) == -1);
	ok1(avl_count(key_regs) == 0);

	ok1(region_map_apply(0, 0, key_regs, &primary) == -1);
	ok1(avl_count(key_regs) == 0);

	ok1(region_map_apply(10, 10, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 0);
	ok1(primary.rows == 10);
	ok1(primary.cols == 10);
	ok1(primary.y == 0);
	ok1(primary.x == 0);

	region_map_key_regs_free(key_regs);

#define TEST1AMT 1 + 3*2 + 6
	diag("----test1----\n#");
	region_map_free();
}

void test2() {
	AVL *key_regs;
	AvlIter i;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2;

	k1 = (void *)1;
	k2 = (void *)2;
	diag("++++test2++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_top(k1, 3);
	region_map_push_top(k2, 14);

	ok1(region_map_apply(1, 1, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 2);
	avl_foreach(i, key_regs) {
		struct aug_region *val = i.value;
		ok1(val->rows == 0);
		ok1(val->cols == 0);
	}
	ok1(primary.rows == 1);
	ok1(primary.cols == 1);
	ok1(primary.y == 0);
	ok1(primary.x == 0);

	region_map_key_regs_clear(&key_regs);

	ok1(region_map_apply(3, 1, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 2);

	avl_foreach(i, key_regs) {
		val = i.value;
		if(i.key != (void *)1) {
			ok1(val->rows == 0);
			ok1(val->cols == 0);
		}
		else {
			ok1(val->rows == 3);
			ok1(val->cols == 1);
			ok1(val->y == 0);
			ok1(val->x == 0);
		}
	}

	ok1(primary.rows == 0);
	ok1(primary.cols == 0);
	region_map_key_regs_clear(&key_regs);

	ok1(region_map_apply(17, 10, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 2);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 3);
	ok1(val->cols == 10);
	ok1(val->y == 0);
	ok1(val->x == 0);	

	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 14);
	ok1(val->cols == 10);
	ok1(val->y == 3);
	ok1(val->x == 0);

	ok1(primary.rows == 0);
	ok1(primary.cols == 0);
	region_map_key_regs_clear(&key_regs);

	ok1(region_map_apply(18, 10, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 2);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 3);
	ok1(val->cols == 10);
	ok1(val->y == 0);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 14);
	ok1(val->cols == 10);
	ok1(val->y == 3);
	ok1(val->x == 0);

	ok1(primary.rows == 1);
	ok1(primary.cols == 10);
	ok1(primary.y == 17);
	ok1(primary.x == 0);
	
	region_map_key_regs_free(key_regs);

#define TEST2AMT 1 + 2+4+4 + 4+6 + 12 + 14
	diag("----test2----\n#");
	region_map_free();
}

void test3() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2, *k3;
	int lines, columns;
	
	lines = 30;
	columns = 40;
	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;
	diag("++++test3++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_top(k1, 3);
	region_map_push_top(k2, 14);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 2);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 3);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 14);
	ok1(val->cols == columns);
	ok1(val->y == 3);
	ok1(val->x == 0);

	ok1(primary.rows == lines-17);
	ok1(primary.cols == columns);
	ok1(primary.y == 17);
	ok1(primary.x == 0);
	
	region_map_key_regs_clear(&key_regs);

	region_map_push_top(k3, 9);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 3);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 3);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 14);
	ok1(val->cols == columns);
	ok1(val->y == 3);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 9);
	ok1(val->cols == columns);
	ok1(val->y == 17);
	ok1(val->x == 0);

	ok1(primary.rows == (lines-26) );
	ok1(primary.cols == columns);
	ok1(primary.y == 26);
	ok1(primary.x == 0);
	
	region_map_key_regs_clear(&key_regs);

	ok1(region_map_delete((void *)5) == -1);
	ok1(region_map_top_size() == 3);
	ok1(region_map_delete(k2) == 0);
	ok1(region_map_top_size() == 2);
	ok1(avl_count(key_regs) == 0);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 2);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 3);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 9);
	ok1(val->cols == columns);
	ok1(val->y == 3);
	ok1(val->x == 0);

	ok1(primary.rows == lines-12);
	ok1(primary.cols == columns);
	ok1(primary.y == 12);
	ok1(primary.x == 0);
	
	region_map_key_regs_clear(&key_regs);

	region_map_push_top(k2, 8);
	ok1(region_map_top_size() == 3);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 3);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 3);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 9);
	ok1(val->cols == columns);
	ok1(val->y == 3);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 8);
	ok1(val->cols == columns);
	ok1(val->y == 12);
	ok1(val->x == 0);

	ok1(primary.rows == lines-20);
	ok1(primary.cols == columns);
	ok1(primary.y == 20);
	ok1(primary.x == 0);
	
	region_map_key_regs_clear(&key_regs);
	
	region_map_key_regs_free(key_regs);
#define TEST3AMT 1 + 2+4*3 + 2+4*4 + 5 + 2+4*3 + 1+2+4*4
	diag("----test3----\n#");
	region_map_free();
}

void test4() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2;
	int lines, columns;
	
	lines = 30;
	columns = 40;
	k1 = (void *)1;
	k2 = (void *)2;

	diag("++++test4++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_top(k1, 3);
	region_map_push_top(k2, 14);
	region_map_push_top(k2, 7);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 2);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 3);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 7);
	ok1(val->cols == columns);
	ok1(val->y == 17);
	ok1(val->x == 0);

	ok1(primary.rows == lines-24);
	ok1(primary.cols == columns);
	ok1(primary.y == 24);
	ok1(primary.x == 0);
	
	region_map_key_regs_free(key_regs);
#define TEST4AMT 1 + 2+4*3
	diag("----test4----\n#");
	region_map_free();
}

void test5() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2, *k3;
	int lines, columns;
	
	lines = 30;
	columns = 40;
	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;

	diag("++++test5++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_top(k1, 10);
	region_map_push_top(k2, 15);
	region_map_push_top(k3, 10);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 3);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 10);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 15);
	ok1(val->cols == columns);
	ok1(val->y == 10);
	ok1(val->x == 0);

	diag("expect that there was no room for k3");
	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 0);
	ok1(val->cols == 0);

	ok1(primary.rows == lines-25);
	ok1(primary.cols == columns);
	ok1(primary.y == 25);
	ok1(primary.x == 0);
	
	region_map_key_regs_free(key_regs);
#define TEST5AMT 1 + 2+4*2+2+4
	diag("----test5----\n#");
	region_map_free();
}

void test6() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2, *k3;
	int lines, columns;
	
	lines = 30;
	columns = 40;
	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;

	diag("++++test6++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_top(k1, 10);
	region_map_push_bot(k2, 15);
	region_map_push_top(k3, 10);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 3);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 10);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 10);
	ok1(val->cols == columns);
	ok1(val->y == 10);
	ok1(val->x == 0);

	diag("expect that there was no room for k2");
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 0);
	ok1(val->cols == 0);

	ok1(primary.rows == lines-20);
	ok1(primary.cols == columns);
	ok1(primary.y == 20);
	ok1(primary.x == 0);
	
	region_map_key_regs_free(key_regs);
#define TEST6AMT 1 + 2+4*2+2+4
	diag("----test6----\n#");
	region_map_free();
}

void test7() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2, *k3;
	int lines, columns;
	
	lines = 30;
	columns = 40;
	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;

	diag("++++test7++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_top(k1, 10);
	region_map_push_bot(k2, 15);
	region_map_push_top(k3, 1);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 3);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 10);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 1);
	ok1(val->cols == columns);
	ok1(val->y == 10);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 15);
	ok1(val->cols == columns);
	ok1(val->y == 15);
	ok1(val->x == 0);

	ok1(primary.rows == lines-26);
	ok1(primary.cols == columns);
	ok1(primary.y == 11);
	ok1(primary.x == 0);
	
	region_map_key_regs_free(key_regs);
#define TEST7AMT 1 + 2+4*4
	diag("----test7----\n#");
	region_map_free();
}

void test8() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2, *k3;
	int lines, columns;
	
	lines = 30;
	columns = 40;
	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;

	diag("++++test8++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_bot(k1, 10);
	region_map_push_bot(k2, 15);
	region_map_push_bot(k3, 4);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 3);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 10);
	ok1(val->cols == columns);
	ok1(val->y == 20);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 15);
	ok1(val->cols == columns);
	ok1(val->y == 5);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 4);
	ok1(val->cols == columns);
	ok1(val->y == 1);
	ok1(val->x == 0);

	ok1(primary.rows == lines-29);
	ok1(primary.cols == columns);
	ok1(primary.y == 0);
	ok1(primary.x == 0);
	
	region_map_key_regs_free(key_regs);
#define TEST8AMT 1 + 2+4*4
	diag("----test8----\n#");
	region_map_free();
}

void test9() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2, *k3, *k4;
	int lines, columns;
	
	lines = 30;
	columns = 40;
	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;
	k4 = (void *)4;

	diag("++++test9++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_bot(k1, 10);
	region_map_push_bot(k2, 9);
	region_map_push_bot(k3, 4);

	region_map_push_top(k4, 3);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 4);

	val = avl_lookup(key_regs, k4);
	ok1(val->rows == 3);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 10);
	ok1(val->cols == columns);
	ok1(val->y == 20);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 9);
	ok1(val->cols == columns);
	ok1(val->y == 11);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 4);
	ok1(val->cols == columns);
	ok1(val->y == 7);
	ok1(val->x == 0);

	ok1(primary.rows == lines-26);
	ok1(primary.cols == columns);
	ok1(primary.y == 3);
	ok1(primary.x == 0);
	
	region_map_key_regs_free(key_regs);
#define TEST9AMT 1 + 2+4*5
	diag("----test9----\n#");
	region_map_free();
}

void test10() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2, *k3, *k4;
	int lines, columns;
	
	lines = 30;
	columns = 40;
	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;
	k4 = (void *)4;

	diag("++++test10++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_bot(k1, 10);
	region_map_push_bot(k2, 9);
	region_map_push_left(k3, 4);

	region_map_push_top(k4, 3);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 4);

	val = avl_lookup(key_regs, k4);
	ok1(val->rows == 3);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 10);
	ok1(val->cols == columns);
	ok1(val->y == 20);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 9);
	ok1(val->cols == columns);
	ok1(val->y == 11);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k3);
	ok1(val->rows == lines-22);
	ok1(val->cols == 4);
	ok1(val->y == 3);
	ok1(val->x == 0);

	ok1(primary.rows == lines-22);
	ok1(primary.cols == columns-4);
	ok1(primary.y == 3);
	ok1(primary.x == 4);
	
	region_map_key_regs_free(key_regs);
#define TEST10AMT 1 + 2+4*5
	diag("----test10----\n#");
	region_map_free();
}

void test11() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2, *k3, *k4, *k5, *k6, *k7;
	int lines, columns;
	
	lines = 30;
	columns = 40;
	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;
	k4 = (void *)4;
	k5 = (void *)5;
	k6 = (void *)6;
	k7 = (void *)7;

	diag("++++test11++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_bot(k1, 5);
	region_map_push_bot(k5, 5);
	region_map_push_bot(k6, 5);

	region_map_push_left(k2, 9);
	region_map_push_left(k3, 4);

	region_map_push_top(k4, 8);
	region_map_push_top(k7, 7);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 7);

	val = avl_lookup(key_regs, k4);
	ok1(val->rows == 8);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);
	val = avl_lookup(key_regs, k7);
	ok1(val->rows == 7);
	ok1(val->cols == columns);
	ok1(val->y == 8);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 5);
	ok1(val->cols == columns);
	ok1(val->y == 25);
	ok1(val->x == 0);
	val = avl_lookup(key_regs, k5);
	ok1(val->rows == 5);
	ok1(val->cols == columns);
	ok1(val->y == 20);
	ok1(val->x == 0);
	val = avl_lookup(key_regs, k6);
	ok1(val->rows == 5);
	ok1(val->cols == columns);
	ok1(val->y == 15);
	ok1(val->x == 0);
	
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 0);
	ok1(val->cols == 0);

	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 0);
	ok1(val->cols == 0);

	ok1(primary.rows == 0);
	ok1(primary.cols == 0);

	ok1(region_map_top_size() == 2);
	ok1(region_map_bot_size() == 3);
	ok1(region_map_left_size() == 2);
	
	region_map_key_regs_free(key_regs);
#define TEST11AMT 1 + 2 + 4*2 + 4*3 + 2*3 + 3
	diag("----test11----\n#");
	region_map_free();
}

void test12() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2, *k3, *k4, *k5, *k6, *k7;
	int lines, columns;
	
	lines = 30;
	columns = 40;
	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;
	k4 = (void *)4;
	k5 = (void *)5;
	k6 = (void *)6;
	k7 = (void *)7;

	diag("++++test12++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_bot(k1, 10);

	region_map_push_left(k5, 10);
	region_map_push_left(k6, 10);
	region_map_push_left(k2, 10);
	region_map_push_left(k3, 10);
	region_map_push_left(k4, 4);

	region_map_push_top(k7, 5);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 7);

	val = avl_lookup(key_regs, k7);
	ok1(val->rows == 5);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 10);
	ok1(val->cols == columns);
	ok1(val->y == 20);
	ok1(val->x == 0);

	
	val = avl_lookup(key_regs, k5);
	ok1(val->rows == lines-15);
	ok1(val->cols == 10);
	ok1(val->y == 5);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k6);
	ok1(val->rows == lines-15);
	ok1(val->cols == 10);
	ok1(val->y == 5);
	ok1(val->x == 10);
	
	val = avl_lookup(key_regs, k2);
	ok1(val->rows == lines-15);
	ok1(val->cols == 10);
	ok1(val->y == 5);
	ok1(val->x == 20);

	val = avl_lookup(key_regs, k3);
	ok1(val->rows == lines-15);
	ok1(val->cols == 10);
	ok1(val->y == 5);
	ok1(val->x == 30);

	val = avl_lookup(key_regs, k4);
	ok1(val->rows == 0);
	ok1(val->cols == 0);

	ok1(primary.rows == 0);
	ok1(primary.cols == 0);

	ok1(region_map_top_size() == 1);
	ok1(region_map_bot_size() == 1);
	ok1(region_map_left_size() == 5);
	
	region_map_key_regs_free(key_regs);
#define TEST12AMT 1 + 2 + 4*2 + 4*4 + 2*2 + 3
	diag("----test12----\n#");
	region_map_free();
}

void test13() {
	AVL *key_regs;
	struct aug_region primary;
	struct aug_region *val;
	void *k1, *k2, *k3, *k4, *k5, *k6, *k7;
	void *k8, *k9, *k10;
	int lines, columns;
	
	lines = 37;
	columns = 130;
	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;
	k4 = (void *)4;
	k5 = (void *)5;
	k6 = (void *)6;
	k7 = (void *)7;
	k8 = (void *)8;
	k9 = (void *)9;
	k10 = (void *)10;

	diag("++++test13++++");	
	region_map_init();
	
	key_regs = region_map_key_regs_alloc();
	ok1(key_regs != NULL);

	region_map_push_top(k1, 3);
	region_map_push_bot(k2, 1);
	region_map_push_bot(k3, 4);
	region_map_push_left(k4, 1);
	region_map_push_left(k5, 10);
	region_map_push_left(k6, 3);
	region_map_push_right(k7, 5);
	region_map_push_right(k8, 2);
	region_map_push_right(k9, 2);
	region_map_push_right(k10, 1);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 10);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 3);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k2);
	ok1(val->rows == 1);
	ok1(val->cols == columns);
	ok1(val->y == lines-1);
	ok1(val->x == 0);

	
	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 4);
	ok1(val->cols == columns);
	ok1(val->y == lines-5);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k4);
	ok1(val->rows == lines-8);
	ok1(val->cols == 1);
	ok1(val->y == 3);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k5);
	ok1(val->rows == lines-8);
	ok1(val->cols == 10);
	ok1(val->y == 3);
	ok1(val->x == 1);

	val = avl_lookup(key_regs, k6);
	ok1(val->rows == lines-8);
	ok1(val->cols == 3);
	ok1(val->y == 3);
	ok1(val->x == 11);

	val = avl_lookup(key_regs, k7);
	ok1(val->rows == lines-8);
	ok1(val->cols == 5);
	ok1(val->y == 3);
	ok1(val->x == columns-5);

	val = avl_lookup(key_regs, k8);
	ok1(val->rows == lines-8);
	ok1(val->cols == 2);
	ok1(val->y == 3);
	ok1(val->x == columns-7);

	val = avl_lookup(key_regs, k9);
	ok1(val->rows == lines-8);
	ok1(val->cols == 2);
	ok1(val->y == 3);
	ok1(val->x == columns-9);

	val = avl_lookup(key_regs, k10);
	ok1(val->rows == lines-8);
	ok1(val->cols == 1);
	ok1(val->y == 3);
	ok1(val->x == columns-10);

	ok1(primary.rows == lines-8);
	ok1(primary.cols == columns-24);
	ok1(primary.y == 3);
	ok1(primary.x == 14);

	ok1(region_map_top_size() == 1);
	ok1(region_map_bot_size() == 2);
	ok1(region_map_left_size() == 3);
	ok1(region_map_right_size() == 4);

	region_map_key_regs_clear(&key_regs);
	
	region_map_delete(k2);
	region_map_delete(k5);
	region_map_delete(k8);

	ok1(region_map_apply(lines, columns, key_regs, &primary) == 0);
	ok1(avl_count(key_regs) == 7);

	val = avl_lookup(key_regs, k1);
	ok1(val->rows == 3);
	ok1(val->cols == columns);
	ok1(val->y == 0);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k3);
	ok1(val->rows == 4);
	ok1(val->cols == columns);
	ok1(val->y == lines-4);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k4);
	ok1(val->rows == lines-7);
	ok1(val->cols == 1);
	ok1(val->y == 3);
	ok1(val->x == 0);

	val = avl_lookup(key_regs, k6);
	ok1(val->rows == lines-7);
	ok1(val->cols == 3);
	ok1(val->y == 3);
	ok1(val->x == 1);

	val = avl_lookup(key_regs, k7);
	ok1(val->rows == lines-7);
	ok1(val->cols == 5);
	ok1(val->y == 3);
	ok1(val->x == columns-5);

	val = avl_lookup(key_regs, k9);
	ok1(val->rows == lines-7);
	ok1(val->cols == 2);
	ok1(val->y == 3);
	ok1(val->x == columns-7);

	val = avl_lookup(key_regs, k10);
	ok1(val->rows == lines-7);
	ok1(val->cols == 1);
	ok1(val->y == 3);
	ok1(val->x == columns-8);

	ok1(primary.rows == lines-7);
	ok1(primary.cols == columns-12);
	ok1(primary.y == 3);
	ok1(primary.x == 4);

	ok1(region_map_top_size() == 1);
	ok1(region_map_bot_size() == 1);
	ok1(region_map_left_size() == 2);
	ok1(region_map_right_size() == 3);


	region_map_key_regs_free(key_regs);
#define TEST13AMT 1 + 2 + 4*11 + 4 + 2 + 4*8 + 4
	diag("----test13----\n#");
	region_map_free();
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
		TESTN(7),
		TESTN(8),
		TESTN(9),
		TESTN(10),
		TESTN(11),
		TESTN(12),
		TESTN(13)
	};

	total_tests = 0;
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

