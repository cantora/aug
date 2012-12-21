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
	AVL *key_dims;
	struct aug_region primary;

	diag("++++test1++++");	
	diag("test basic functionality/sanity");
	region_map_init();
	
	key_dims = region_map_key_dims_alloc();
	ok1(key_dims != NULL);

	ok1(region_map_dims(0, 10, key_dims, &primary) == -1);
	ok1(avl_count(key_dims) == 0);

	ok1(region_map_dims(1234, 0, key_dims, &primary) == -1);
	ok1(avl_count(key_dims) == 0);

	ok1(region_map_dims(0, 0, key_dims, &primary) == -1);
	ok1(avl_count(key_dims) == 0);

	ok1(region_map_dims(10, 10, key_dims, &primary) == 0);
	ok1(avl_count(key_dims) == 0);
	ok1(primary.rows == 10);
	ok1(primary.cols == 10);
	ok1(primary.y == 0);
	ok1(primary.x == 0);

	region_map_key_dims_free(key_dims);

#define TEST1AMT 1 + 3*2 + 6
	diag("----test1----\n#");
	region_map_free();
}

void test2() {
	AVL *key_dims;
	AvlIter i;
	struct aug_region primary;
	void *k1, *k2, *k3;

	k1 = (void *)1;
	k2 = (void *)2;
	k3 = (void *)3;
	diag("++++test2++++");	
	region_map_init();
	
	key_dims = region_map_key_dims_alloc();
	ok1(key_dims != NULL);

	region_map_push_top(k1, 3);
	region_map_push_top(k2, 14);

	ok1(region_map_dims(1, 1, key_dims, &primary) == 0);
	ok1(avl_count(key_dims) == 2);
	avl_foreach(i, key_dims) {
		struct aug_region *val = i.value;
		ok1(val->rows == 0);
		ok1(val->cols == 0);
	}
	ok1(primary.rows == 0);
	ok1(primary.cols == 0);
	region_map_key_dims_clear(key_dims);

	ok1(region_map_dims(3, 1, key_dims, &primary) == 0);
	ok1(avl_count(key_dims) == 2);

	avl_foreach(i, key_dims) {
		int k = i.key;
		struct aug_region *val = i.value;
		if(k != 1) {
			ok1(val->rows == 0);
			ok1(val->cols == 0);
		}
		else {
			ok1(val->rows == 3);
			ok1(val->cols == 1);
		}
	}

	ok1(primary.rows == 0);
	ok1(primary.cols == 0);
	region_map_key_dims_clear(key_dims);

	region_map_key_dims_free(key_dims);

#define TEST2AMT 1 + 4+4 + 4+4
	diag("----test2----\n#");
	region_map_free();
}


int main()
{
	int i, len, total_tests;
#define TESTN(_num) {test##_num, TEST##_num##AMT}
	struct aug_test tests[] = {
		TESTN(1),
		TESTN(2)
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

