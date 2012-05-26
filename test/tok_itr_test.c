#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "tok_itr.h"
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
	char val[32];
	struct aug_tok_itr itr;
	int i;

	diag("++++test1++++");
#	define TOKALL TOK1 ":" TOK2 ":" TOK3 ":" TOK4
	strncpy(str, TOKALL, 128);
	//printf("str: %s\n", str);
	diag("test token iterator basics");

	i = 0;
	for(tok_itr_init(&itr, str, ':'); !tok_itr_end(&itr); tok_itr_next(&itr) ) {
		ok1( tok_itr_val(&itr, val, 32) == 0 );	
		//printf("%d: '%s'\t(%s) %d\n", i, val, str, strlen(arr[i]) );
		ok1( strcmp(val, arr[i++]) == 0 );
	}	
	
	diag("test whether iterator modified the string it iterated over");
	ok1( strcmp(str, TOKALL) == 0 );

#define TEST1AMT 2*4 + 1
	diag("----test1----\n#");

#	undef TOKALL
#	undef TOK1
#	undef TOK2
#	undef TOK3
#	undef TOK4
}

void test2() {
#	define TOK1 "blah"
#	define TOK2 "@#$%zlk"
#	define TOK3 "(*aksjhdn$"
#	define TOK4 "\"///\\zxkkkkkhs)(*&"
	const char *arr[] = {TOK1, "", TOK2, "", "", "", TOK3, "", TOK4 };
	char str[128];
	char val[32];
	struct aug_tok_itr itr;
	int i;

	diag("++++test2++++");
#	define TOKALL TOK1 "::" TOK2 "::::" TOK3 "::" TOK4
	strncpy(str, TOKALL, 128);
	//printf("str: %s\n", str);
	diag("test token iterator with empty tokens");

	i = 0;
	for(tok_itr_init(&itr, str, ':'); !tok_itr_end(&itr); tok_itr_next(&itr) ) {
		ok1( tok_itr_val(&itr, val, 32) == 0 );	
		//printf("%d: '%s'\t(%s) %d\n", i, val, str, strlen(arr[i]) );
		ok1( strcmp(val, arr[i++]) == 0 );
	}	
	
	diag("test whether iterator modified the string it iterated over");
	ok1( strcmp(str, TOKALL) == 0 );

#define TEST2AMT 2*9 + 1
	diag("----test2----\n#");

#	undef TOKALL
#	undef TOK1
#	undef TOK2
#	undef TOK3
#	undef TOK4
}

void test3() {
#	define TOK1 "blah"
#	define TOK2 "@#$%zlk"
#	define TOK3 "(*aksjhdn$"
#	define TOK4 "\"///\\zxkkkkkhs)(*&"
	const char *arr[] = {"", "", TOK1, "", TOK2, "", "", "", TOK3, "", TOK4, "", "", ""};
	char str[128];
	char val[32];
	struct aug_tok_itr itr;
	int i;

	diag("++++test3++++");
#define TOKALL "::" TOK1 "::" TOK2 "::::" TOK3 "::" TOK4 ":::"
	strncpy(str, TOKALL, 128);
	//printf("str: %s\n", str);
	diag("test token iterator with leading an trailing empty tokens");

	i = 0;
	for(tok_itr_init(&itr, str, ':'); !tok_itr_end(&itr); tok_itr_next(&itr) ) {
		ok1( tok_itr_val(&itr, val, 32) == 0 );	
		//printf("%d: '%s'\t(%s) %d\n", i, val, str, strlen(arr[i]) );
		ok1( strcmp(val, arr[i++]) == 0 );
	}	
	
	diag("test whether iterator modified the string it iterated over");
	ok1( strcmp(str, TOKALL) == 0 );

#define TEST3AMT 2*14 + 1
	diag("----test3----\n#");

#	undef TOKALL
#	undef TOK1
#	undef TOK2
#	undef TOK3
#	undef TOK4
}

void test4() {
	char tok1[64];
	char tok2[64];
	char tok3[64];
	char tok4[64];
	char tok5[64];
	const char *arr[] = {tok1, tok2, tok3, tok4, tok5, ""};
	char str[512];
	char val[32];
	struct aug_tok_itr itr;
	int i;

	diag("++++test4++++");
	
	str[0] = '\0';

#	define TOK_SET(_str, _chr, _amt) \
		memset(_str, _chr, _amt); \
		_str[_amt] = ':'; \
		_str[_amt+1] = '\0';

	TOK_SET(tok1, 'A', 29);
	TOK_SET(tok2, 'B', 30);
	TOK_SET(tok3, 'C', 31);
	TOK_SET(tok4, 'D', 32);
	TOK_SET(tok5, 'E', 33);

	for(i = 0; i < 5; i++) {
		//printf("tok%d: %s\n", i+1, arr[i]);
		strcat(str, arr[i]);
	}

	//printf("str: %s\n", str);
	diag("test token iterator with big tokens");

	i = 0;
	for(tok_itr_init(&itr, str, ':'); !tok_itr_end(&itr); tok_itr_next(&itr) ) {
		int status = tok_itr_val(&itr, val, 32);
		int len = strlen(arr[i]);
		int cmplen;
		
		if(arr[i][len-1] == ':')
			len--;

		cmplen = len;
		if(len > 31) {
			diag("tok4 and tok5 should be too big (size > 31)");
			ok1(status != 0);	
			cmplen = 31;
		}
		else
			ok1(status == 0);

		//printf("%d: '%s'\t(%s) %d\n", i, val, str, strlen(arr[i]) );
		ok1( strncmp(val, arr[i++], cmplen) == 0 );
	}	

#define TEST4AMT 2*6
	diag("----test4----\n#");

#	undef TOK_SET
}

void test5() {
#	define NUM_DELIMS 48
#	define NUM_TOKENS (NUM_DELIMS + 1)
	char arr[NUM_TOKENS][1024];;
	char str[1024];
	char val[1024];
	struct aug_tok_itr itr;
	int i;
	char delim = ':';
	char *cptr;

	srand(time(NULL));

	diag("++++test5++++");
	
	for(i = 0; i < 1023; i++) {
		char chr;
		do {
			chr = (rand() % 256);
		} while(chr == delim || chr == '\0');
		str[i] = chr;
	}
	str[i] = '\0';

	for(i = 0; i < NUM_DELIMS; i++) {
		int pos;
		do {
			pos = rand() % 1023; 
		} while(str[pos] == '\0');		
		str[pos] = '\0';
	}
	
	i = 0;
	for(cptr = str; cptr < (str+1024); cptr += (strlen(cptr)+1) ) {
		assert(i < NUM_TOKENS);
		strcpy(arr[i], cptr);
		//printf("%d: %s\n", i, arr[i]);
		i++;
	}

	for(i = 0; i < 1023; i++)
		if(str[i] == '\0')
			str[i] = delim;

	//printf("str: %s\n", str);
	diag("test token iterator with random token string");
	
	i = 0;
	for(tok_itr_init(&itr, str, delim); !tok_itr_end(&itr); tok_itr_next(&itr) ) {
		ok1( tok_itr_val(&itr, val, 1024) == 0 );
		ok1( strcmp(val, arr[i++]) == 0 );
		//printf("val: %s\n", val);
	}	

#define TEST5AMT 2*NUM_TOKENS
	diag("----test5----\n#");
}

void test6() {
#	define TOK1 "blah"
#	define TOK2 "@#$%zlk"
#	define TOK3 "(*aksjhdn$"
#	define TOK4 "\"///\\zxkkkkkhs)(*&"
	const char *arr[] = {TOK1, TOK2, TOK3, TOK4 };
	char str[128];
	char val[32];
	int i;
	TOK_ITR_USE_FOREACH_FUNCTION();

	diag("++++test6++++");
#	define TOKALL TOK1 ":" TOK2 ":" TOK3 ":" TOK4
	strncpy(str, TOKALL, 128);
	//printf("str: %s\n", str);
	diag("test token iterator FOREACH basics");

	i = 0;
	TOK_ITR_FOREACH(val, 32, str, ':') {
		ok1( strcmp(val, arr[i++]) == 0 );
	}	
	
	diag("test whether iterator modified the string it iterated over");
	ok1( strcmp(str, TOKALL) == 0 );

#define TEST6AMT 4 + 1
	diag("----test6----\n#");

#	undef TOKALL
#	undef TOK1
#	undef TOK2
#	undef TOK3
#	undef TOK4
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
		TESTN(6)
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

