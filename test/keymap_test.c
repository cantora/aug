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
#include "keymap.h"

struct aug_test {
	void (*fn)();
	int amt;
};

static uint32_t test1chr = '1';
static void * test1user = &test1chr;

void test1_cb1(uint32_t chr, void *user) {
	diag("test1_cb1: enter");
	ok1(chr == test1chr);
	ok1(user == test1user);
	diag("test1_cb1: exit");
}

void test1() {
	struct aug_keymap map;
	keymap_init(&map);
	aug_on_key_fn on_key;
	void *user;

	diag("++++test1++++");	
	diag("test basic functionality/sanity");
	keymap_bind(&map, test1chr, test1_cb1, test1user);
	
	on_key = NULL;
	user = NULL;
	keymap_binding(&map, test1chr, &on_key, &user);
	ok1(on_key != NULL);
	ok1(user != NULL);

	(*on_key)(test1chr, user);

	ok1(keymap_size(&map) == 1);

#define TEST1AMT 2 + 2 + 1
	diag("----test1----\n#");

	keymap_free(&map);
}

static uint32_t test2chr1 = '2';
static void * test2user1 = (void *) 0x0021;

void test2_cb1(uint32_t chr, void *user) {
	diag("test2_cb1: enter");
	ok1(chr == test2chr1);
	ok1(user == test2user1);
	diag("test2_cb1: exit");
}

static void * test2user2 = (void *) 0x0022;

void test2_cb2(uint32_t chr, void *user) {
	diag("test2_cb2: enter");
	ok1(chr == test2chr1);
	ok1(user == test2user2);
	diag("test2_cb2: exit");
}

void test2() {
	struct aug_keymap map;
	aug_on_key_fn on_key;
	void *user;

	keymap_init(&map);
	diag("++++test2++++");	
	diag("binding an already bound key: should overwrite.");
	diag("bind key to first function...");
	keymap_bind(&map, test2chr1, test2_cb1, test2user1);
	
	on_key = NULL;
	user = NULL;
	keymap_binding(&map, test2chr1, &on_key, &user);
	ok1(on_key != NULL);
	ok1(user != NULL);

	(*on_key)(test2chr1, user);

	ok1(keymap_size(&map) == 1);

	diag("bind key to a different function...");
	keymap_bind(&map, test2chr1, test2_cb2, test2user2);
	ok1(keymap_size(&map) == 1);

	diag("do we get the new function or old?");
	on_key = NULL;
	user = NULL;
	keymap_binding(&map, test2chr1, &on_key, &user);
	ok1(on_key != NULL);
	ok1(user != NULL);

	ok1(on_key == test2_cb2);
	(*on_key)(test2chr1, user);
	
#define TEST2AMT 2 + 2 + 1 + 1 + 2 + 1 + 2
	diag("----test2----\n#");

	keymap_free(&map);
}

static uint32_t test3chr = '3';
static void * test3user = &test3chr;

void test3_cb1(uint32_t chr, void *user) {
	diag("test3_cb1: enter");
	ok1(chr == test3chr);
	ok1(user == test3user);
	diag("test3_cb1: exit");
}

void test3() {
	struct aug_keymap map;
	keymap_init(&map);
	aug_on_key_fn on_key;
	void *user;

	diag("++++test3++++");	
	diag("does unbind work?");
	keymap_bind(&map, test3chr, test3_cb1, test3user);
	
	on_key = NULL;
	user = NULL;
	keymap_binding(&map, test3chr, &on_key, &user);
	ok1(on_key != NULL);
	ok1(user != NULL);

	(*on_key)(test3chr, user);

	ok1(keymap_size(&map) == 1);
	
	diag("unbind an non-existent key");
	ok1(keymap_unbind(&map, '4') != 0);
	ok1(keymap_size(&map) == 1);

	diag("unbind the actual key");
	ok1(keymap_unbind(&map, test3chr) == 0);
	ok1(keymap_size(&map) == 0);

	diag("asking for the binding just returns null values");
	on_key = NULL;
	user = NULL;
	keymap_binding(&map, test3chr, &on_key, &user);
	ok1(on_key == NULL);
	ok1(user == NULL);

#define TEST3AMT 2 + 2 + 1 + 2 + 2 + 2
	diag("----test3----\n#");

	keymap_free(&map);
}

int main()
{
	int i, len, total_tests;
#define TESTN(_num) {test##_num, TEST##_num##AMT}
	struct aug_test tests[] = {
		TESTN(1),
		TESTN(2),
		TESTN(3)
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

