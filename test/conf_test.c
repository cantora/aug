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
#include <string.h>
#include <err.h>
#include "conf.h"
#include "util.h"
#include "opt.h"
#include <ccan/tap/tap.h>

struct conf_test {
	void (*fn)();
	int amt;
};

struct addr_name_pair {
	void *addr;
	const char *name;
};

int g_argc;
char **g_argv;
struct aug_conf g_default_conf;

int compare_conf_vals(struct aug_conf *c1, struct aug_conf *c2) {
#define ccv_CMP(_var) if( memcmp(&c1->_var, &c2->_var, sizeof(c1->_var) ) != 0) return -1

	ccv_CMP(nocolor);
	ccv_CMP(ncterm);
	ccv_CMP(term);
	ccv_CMP(debug_file);
	ccv_CMP(conf_file);
#undef ccv_CMP
	return 0;
}

#define AUG_CONF_PAIRS(_conf_ptr) \
	{&_conf_ptr->nocolor, "nocolor"}, \
	{&_conf_ptr->ncterm, "ncterm"}, \
	{&_conf_ptr->term, "term"}, \
	{&_conf_ptr->debug_file, "debug_file"}, \
	{&_conf_ptr->conf_file, "conf_file"}

int opt_set_amt(struct aug_conf *c) {
	struct addr_name_pair addr[] = { AUG_CONF_PAIRS(c) };
	int i, len, amt;

	len = AUG_ARRAY_SIZE(addr);

	amt = 0;
	for(i = 0; i < len; i++) {
		//printf("%d\t", i); fflush(stdout); 
		//printf("check for %s\n", addr[i].name);
		if(objset_get(&c->opt_set, addr[i].addr) ) {
			amt++;
		}
	}

	return amt;
}

void test1() {
	struct aug_conf c;
	int argc = 1;
	char *argv[] = {g_argv[0], NULL};

	conf_init(&c);
	diag("no ini file and no args");
	ok1(opt_parse(argc, argv, &c) == 0);
	ok1(opt_set_amt(&c) == 0);
	ok1( compare_conf_vals(&c, &g_default_conf) == 0);

	conf_free(&c);
#define TEST1AMT 3
}

void test2() {
	struct aug_conf c;
	int argc = 1;
	char *argv[] = {g_argv[0], NULL};
	const char *path = "/tmp/augrc";
	dictionary *ini;
	FILE *f = fopen(path, "w");
	if(f == NULL)
		err(1, "file: %s", path);
	fclose(f);

	diag("blank ini file and no args");
	ini = ciniparser_load(path);
	ok1(ini != NULL);

	conf_init(&c);
	ok1(opt_parse(argc, argv, &c) == 0);
	ok1(opt_set_amt(&c) == 0);
	conf_merge_ini(&c, ini);	
	ok1( compare_conf_vals(&c, &g_default_conf) == 0);

	conf_free(&c);
#define TEST2AMT 4
	
	ciniparser_freedict(ini);
	unlink(path);
}

void test3() {
	struct aug_conf c;
	int argc = 1;
	char *argv[] = {g_argv[0], NULL};
	const char *path = "/tmp/augrc";
	dictionary *ini;
	const char *ncterm;
	FILE *f = fopen(path, "w");
	if(f == NULL)
		err(1, "file: %s", path);

	fprintf(f, "[" CONF_CONFIG_SECTION_CORE "]\n\n");
	fprintf(f, "ncterm = screen ;\n");
	fclose(f);
	diag("ini file overrides default");
	ini = ciniparser_load(path);
	ok1(ini != NULL);

	conf_init(&c);
	ok1(opt_parse(argc, argv, &c) == 0);
	ok1(opt_set_amt(&c) == 0);
	ok1( compare_conf_vals(&c, &g_default_conf) == 0);
	ncterm = NULL;
	ncterm = ciniparser_getstring(ini, CONF_CONFIG_SECTION_CORE ":ncterm", NULL);
	ok1( ncterm != NULL);
	if(ncterm == NULL)
		return;
	ok1( strcmp(ncterm, "screen") == 0 );

	conf_merge_ini(&c, ini);
	ok1( c.ncterm != NULL );
	if(c.ncterm == NULL)
		return;
	ok1( strcmp(c.ncterm, "screen") == 0 );
	c.ncterm = CONF_NCTERM_DEFAULT;
	ok1( compare_conf_vals(&c, &g_default_conf) == 0);
	
	conf_free(&c);
#define TEST3AMT 9

	ciniparser_freedict(ini);
	unlink(path);
}

void test4() {
	struct aug_conf c;
	char *argv[] = {g_argv[0], "--ncterm", "xterm", NULL};
	int argc = AUG_ARRAY_SIZE(argv) - 1;

	const char *path = "/tmp/augrc";
	dictionary *ini;
	const char *ncterm;
	FILE *f = fopen(path, "w");
	if(f == NULL)
		err(1, "file: %s", path);

	fprintf(f, "[" CONF_CONFIG_SECTION_CORE "]\n\n");
	fprintf(f, "ncterm = screen ;\n");
	fclose(f);

	diag("command line overrides ini file");
	ini = ciniparser_load(path);
	ok1(ini != NULL);

	conf_init(&c);
	ok1(opt_parse(argc, argv, &c) == 0);
	ok1(opt_set_amt(&c) == 1);
	ncterm = NULL;
	ncterm = ciniparser_getstring(ini, CONF_CONFIG_SECTION_CORE ":ncterm", NULL);
	ok1( ncterm != NULL);
	if(ncterm == NULL)
		return;
	ok1( strcmp(ncterm, "screen") == 0 );

	conf_merge_ini(&c, ini);
	ok1( c.ncterm != NULL );
	ok1( strcmp(c.ncterm, "xterm") == 0 );
	c.ncterm = CONF_NCTERM_DEFAULT;
	ok1( compare_conf_vals(&c, &g_default_conf) == 0);
	
#define TEST4AMT 8
	conf_free(&c);
	ciniparser_freedict(ini);
	unlink(path);
}

int main(int argc, char *argv[])
{
	int i, len, total_tests;
#define TESTN(_num) {test##_num, TEST##_num##AMT}
	struct conf_test tests[] = {
		TESTN(1),
		TESTN(2),
		TESTN(3),
		TESTN(4)
	};

	g_argc = argc;
	g_argv = argv;
	conf_init(&g_default_conf);

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

