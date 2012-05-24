#include <stdio.h>
#include <errno.h>
#include "opt.h"
#include "util.h"

void print_conf(struct aug_conf *c) {
	int i;

	printf("nocolor: \t\t'%d'\n", c->nocolor);
	printf("ncterm: \t\t'%s'\n", c->ncterm);
	printf("term: \t\t\t'%s'\n", c->term);
	printf("debug_file: \t\t'%s'\n", c->debug_file);
	printf("conf_file: \t\t'%s'\n", c->conf_file);
	printf("cmd: \t\t\t[");
	for(i = 0; i < c->cmd_argc; i++) {
		printf("'%s'", c->cmd_argv[i]);
		if(i < c->cmd_argc - 1)
			printf(", ");
	}
	printf("]\n");
}

struct addr_name_pair {
	void *addr;
	const char *name;
};

void print_opt_set(struct aug_conf *c) {
	struct addr_name_pair addr[] = { {&c->nocolor, "nocolor"}, {&c->ncterm, "ncterm"}, 
										{&c->term, "term"}, {&c->debug_file, "debug_file"},
										{&c->conf_file, "conf_file"} };
	int i, len, amt;

	len = AUG_ARRAY_SIZE(addr);

	amt = 0;
	for(i = 0; i < len; i++) {
		if(objset_get(&c->opt_set, addr[i].addr) ) {
			printf("%s option set\n", addr[i].name);
			amt++;
		}

	}

	printf("%d options set on command line\n", amt);	
}

int main(int argc, char *argv[]) {
	struct aug_conf c;

	conf_init(&c);
	
	opt_print_help(argc, argv);
	printf("DEFAULT:\n");
	print_conf(&c);

	if(opt_parse(argc, argv, &c) != 0) {
		switch(errno) {
		case OPT_ERR_HELP:
		case OPT_ERR_USAGE:
			goto parsed;
			break;
		case OPT_ERR_MISSING_ARG:
			printf("error: missing arg\n");
			break;
		case OPT_ERR_UNKNOWN_OPTION:
			printf("error: unknown option\n");
			break;
		}
	
		printf("error msg: %s\n", opt_err_msg);
		goto done;
	}

parsed:	
	printf("AFTER PARSING:\n");
	print_conf(&c);
	print_opt_set(&c);
done:	
	return 0;
}