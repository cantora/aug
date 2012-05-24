/* 
 * Copyright 2012 anthony cantor
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

#include "opt.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>

#define AUG_QUOTE(_EXP) #_EXP
#define AUG_EXPAND_QUOTE(_EXP) AUG_QUOTE(_EXP)

struct opt_desc {
	const char *usage;
	const char *const desc[3];
	const struct option lopt; 	/* getopt long option struct */
};

char opt_err_msg[64];

#define LONG_ONLY_VAL(_index) ((1 << 11) + _index)

/* when adding a new option:
 *    1. insert, append or prepend an opt_desc struct to the array below
 *    2. make sure to create the defines:
 *              OPT_* "somename"
 *              OPT_*_INDEX (OPT_(previous opt)_INDEX+1)
 *    3. change opt_parse so that the option switch statement recognizes 
 *       the value in .lopt.val
 *	  4. change opt_init so that the default value gets initialized.
 */
static const struct opt_desc aug_options[] = {
	{
#define OPT_CONFIG "config"
#define OPT_CONFIG_INDEX 0
		.usage = " FILEPATH",
		.desc = {"specify the path of the configuration file", 
					"\tdefault: " CONF_CONFIG_FILE_DEFAULT, NULL},
		.lopt = {OPT_CONFIG, 1, 0, 'c'}
	},
	{
#define OPT_NOCOLOR "no-color"
#define OPT_NOCOLOR_INDEX (OPT_CONFIG_INDEX+1)
		.usage = NULL,
		.desc = {"turns off color support", NULL},
		.lopt = {OPT_NOCOLOR, 0, 0, LONG_ONLY_VAL(OPT_NOCOLOR_INDEX)}
	},
	{
#define OPT_TERM CONF_TERM
#define OPT_TERM_INDEX (OPT_NOCOLOR_INDEX+1)
		.usage = " TERMNAME",
		.desc = {"sets the TERM environment variable to TERMNAME", 
				 "\tdefault: the current value of TERM in the environment", NULL},
		.lopt = {OPT_TERM, 1, 0, LONG_ONLY_VAL(OPT_TERM_INDEX)}
	},
	{
#define OPT_NCTERM CONF_NCTERM
#define OPT_NCTERM_INDEX (OPT_TERM_INDEX+1)
		.usage = " TERMNAME",
		.desc = {"sets the terminal profile used by ncurses",
				 "\tdefault: the current value of TERM in the environment", NULL},

		.lopt = {OPT_NCTERM, 1, 0, LONG_ONLY_VAL(OPT_NCTERM_INDEX)}
	},
	{
#define OPT_DEBUG CONF_DEBUG_FILE
#define OPT_DEBUG_INDEX (OPT_NCTERM_INDEX+1)
		.usage = " FILEPATH",
		.desc = {"collect debug messages into FILENAME", NULL},
		.lopt = {OPT_DEBUG, 1, 0, 'd'}
	},
	{
#define OPT_HELP "help"
#define OPT_HELP_INDEX (OPT_DEBUG_INDEX+1)
		.usage = NULL,
		.desc = {"display this message", NULL},
		.lopt = {OPT_HELP, 0, 0, 'h'}
	}
}; /* aug_options */
#define AUG_OPTLEN (OPT_HELP_INDEX+1)

static void init_long_options(struct option *long_options, char *optstring) {
	int i, os_i;

	os_i = 0;
	optstring[os_i++] = '+'; /* stop processing after first non-option */
	optstring[os_i++] = ':'; /* return : for missing arg */

	for(i = 0; i < AUG_OPTLEN; i++) {
		long_options[i] = aug_options[i].lopt;
		if(aug_options[i].lopt.val > 0 && aug_options[i].lopt.val < 256) {
			optstring[os_i++] = aug_options[i].lopt.val;
			switch(aug_options[i].lopt.has_arg) {
			case 2:
				optstring[os_i++] = ':';
				/* fall through */
			case 1: 
				optstring[os_i++] = ':';
				break;
			case 0:
				break; /* do nothing */
			default:
				assert(0); /* shouldnt get here */
			}				
		}
	}
	
	long_options[i].name = NULL;
	long_options[i].has_arg = 0;
	long_options[i].flag = NULL;
	long_options[i].val = 0;	
	optstring[os_i++] = '\0'; 
}


void opt_print_usage(int argc, const char *const argv[]) {
	fprintf(stdout, "usage: %s [OPTIONS] [CMD [ARG1, ...]]\n", (argc > 0)? argv[0] : "");
}

void opt_print_help(int argc, const char *const argv[]) {
	int i, k, f1_amt;
#define F1_SIZE 32
#define F1_WIDTH 26
	char f1[F1_SIZE];
	
	opt_print_usage(argc, argv);
	
	fprintf(stdout, "\n%-" AUG_EXPAND_QUOTE(F1_WIDTH) "s%s\n", "  CMD", "run CMD with arguments instead of the default");
	fprintf(stdout, "%-" AUG_EXPAND_QUOTE(F1_WIDTH) "s\tdefault: the value of SHELL in the environment\n", "");
	fprintf(stdout, "%-" AUG_EXPAND_QUOTE(F1_WIDTH) "s\tor if SHELL undefined: ", "");
	for(k = 0; k < CONF_DEFAULT_ARGC; k++)
		fprintf(stdout, "%s ", CONF_DEFAULT_ARGV[k]);
	
	fprintf(stdout, "\nOPTIONS:\n");
	for(i = 0; i < AUG_OPTLEN; i++) {
		f1_amt = 0;
		f1_amt += snprintf(f1, F1_SIZE, "  --%s", aug_options[i].lopt.name);
		if(f1_amt < F1_SIZE && aug_options[i].lopt.val >= 0 && aug_options[i].lopt.val < 256) 
			f1_amt += snprintf(f1+f1_amt, F1_SIZE-f1_amt, "|-%c", aug_options[i].lopt.val);
		
		if(f1_amt < F1_SIZE && aug_options[i].lopt.has_arg && aug_options[i].usage != NULL)
			snprintf(f1+f1_amt, F1_SIZE-f1_amt, "%s", aug_options[i].usage);

		/*desc = aug_options[i].desc;
		do {
			fprintf(stdout, "%-" AUG_EXPAND_QUOTE(F1_WIDTH) "s%s\n", f1, *desc);
		} while( *(desc++) != NULL );*/
		for(k = 0; aug_options[i].desc[k] != NULL; k++) 
			fprintf(stdout, "%-" AUG_EXPAND_QUOTE(F1_WIDTH) "s%s\n", (k == 0? f1 : ""), aug_options[i].desc[k]);
	}
	
	fputc('\n', stdout);
}

int opt_parse(int argc, char *const argv[], struct aug_conf *conf) {
	int index, c;
	char optstring[64];
	struct option long_options[AUG_OPTLEN+1];
		
	/* dont print error message */
	opterr = 0;
	init_long_options(long_options, optstring);
	
	index = 0;
	while( (c = getopt_long(argc, argv, optstring, long_options, &index)) != -1 ) {
		if(optarg && optarg[0] == '-') {
			snprintf(opt_err_msg, 64, "option '%s' missing argument", argv[optind-2]);
			errno = OPT_ERR_MISSING_ARG;
			goto fail;
		}
			
#define OPT_SET(_var, _value) objset_add(&conf->opt_set, &_var); _var = _value
		switch (c) {
		case 'd': /* debug file */
			OPT_SET(conf->debug_file, optarg);
			break;
	
		case 'c': /* configuration file */
			OPT_SET(conf->conf_file, optarg);
			break;
	
		case LONG_ONLY_VAL(OPT_NOCOLOR_INDEX):
			OPT_SET(conf->nocolor, 1);
			break;

		case LONG_ONLY_VAL(OPT_TERM_INDEX):
			OPT_SET(conf->term, optarg);
			break;

		case LONG_ONLY_VAL(OPT_NCTERM_INDEX):
			OPT_SET(conf->ncterm, optarg);
			break;

#undef OPT_SET
		case 'h':
			errno = OPT_ERR_HELP;
			goto fail;
			break;
			
		case '?':
			snprintf(opt_err_msg, 64, "unknown option '%s'", argv[optind-1]);
			errno = OPT_ERR_UNKNOWN_OPTION;
			goto fail;
			break;
		
		case ':': 
			snprintf(opt_err_msg, 64, "option '%s' missing argument", argv[optind-1]);
			errno = OPT_ERR_MISSING_ARG;
			goto fail;
			break;

		default:
			printf("getopt returned char '%c' (%d)\n", c, c);
			assert(0); /* shouldnt get here */
		}
		
	}

	/* non-default command was passed (as non-option args) */
	if(optind < argc) {
		conf->cmd_argc = argc - optind;
		conf->cmd_argv = (const char *const *) &argv[optind];
	}	

	return 0;
fail:
	return -1;			   
}
