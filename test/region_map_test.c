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

	region_map_key_regs_clear(key_regs);

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
	region_map_key_regs_clear(key_regs);

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
	region_map_key_regs_clear(key_regs);

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
	
	region_map_key_regs_clear(key_regs);

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
	
	region_map_key_regs_clear(key_regs);

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
	
	region_map_key_regs_clear(key_regs);

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
	
	region_map_key_regs_clear(key_regs);

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
	
	region_map_key_regs_clear(key_regs);
	
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

int main()
{
	int i, len, total_tests;
#define TESTN(_num) {test##_num, TEST##_num##AMT}
	struct aug_test tests[] = {
		TESTN(1),
		TESTN(2),
		TESTN(3),
		TESTN(4),
		TESTN(5)
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

