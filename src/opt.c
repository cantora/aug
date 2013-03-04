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
#include <errno.h>
#include <assert.h>
#include "screen.h"
#include "err.h"

#define AUG_QUOTE(_EXP) #_EXP
#define AUG_EXPAND_QUOTE(_EXP) AUG_QUOTE(_EXP)

struct opt_desc {
	const char *usage;
	const char *const desc[8];
	const struct option lopt; 	/* getopt long option struct */
};

char opt_err_msg[64];

#define LONG_ONLY_VAL(_index) ((1 << 11) + _index)

/* when adding a new option:
 *    1. insert, append or prepend an opt_desc struct to the array below
 *    2. make sure to create the defines:
 *              OPT_* "somename"
 *              OPT_*_INDEX (OPT_(previous opt)_INDEX+1)
 *	  3. if necessary, add an element to aug_conf in conf.h and add a line
 *		 to conf_init so that the default value gets initialized.
 *    4. change opt_parse so that the option switch statement recognizes 
 *       the value in .lopt.val and sets aug_conf accordingly.
 */
const struct opt_desc AUG_OPTIONS[] = {
	{
#define OPT_CONFIG "config"
#define OPT_CONFIG_INDEX 0
		.usage = " FILEPATH",
		.desc = {"specify the path of the configuration file.", 
					"\tdefault: " CONF_CONFIG_FILE_DEFAULT, NULL},
		.lopt = {OPT_CONFIG, 1, 0, 'c'}
	},
	{
#define OPT_CMD_PREFIX_USAGE " <char>"
#define OPT_CMD_PREFIX CONF_CMD_PREFIX
#define OPT_CMD_PREFIX_INDEX (OPT_CONFIG_INDEX+1)
		.usage = OPT_CMD_PREFIX_USAGE,
		.desc = {"set prefix key for core commands and plugin extension commands.",
					"\tsee option --char-rep for a list of valid character representation",
					"\tstrings that can be passed in as an argument here.",
					"\tdefault: " CONF_CMD_PREFIX_DEFAULT, NULL},
		.lopt = {OPT_CMD_PREFIX, 1, 0, 'e' }
	},
	{
#define OPT_CMD_PREFIX_ESCAPE CONF_CMD_PREFIX_ESCAPE
#define OPT_CMD_PREFIX_ESCAPE_INDEX (OPT_CMD_PREFIX_INDEX+1)
		.usage = OPT_CMD_PREFIX_USAGE,
		.desc = {"set prefix key escape extension, i.e. if set to 'a' then typing",
					"\t^A-a will output a literal ^A to the terminal.",
					"\tsee man page for details on 'pass through' mode.", 
					"\tdefault: 'pass through' mode.", NULL},
		.lopt = {OPT_CMD_PREFIX_ESCAPE, 1, 0, 'E' }
	},
	{
#define OPT_CHAR_REP "char-rep"
#define OPT_CHAR_REP_INDEX (OPT_CMD_PREFIX_ESCAPE_INDEX+1)
		.usage = NULL,
		.desc = {"show a list of character representation strings which",
					"\tare valid arguments for -e and -E.", NULL},
		.lopt = {OPT_CHAR_REP, 0, 0, LONG_ONLY_VAL(OPT_CHAR_REP_INDEX)}
	},
	{
#define OPT_NOCOLOR "no-color"
#define OPT_NOCOLOR_INDEX (OPT_CHAR_REP_INDEX+1)
		.usage = NULL,
		.desc = {"turn off color support.", NULL},
		.lopt = {OPT_NOCOLOR, 0, 0, LONG_ONLY_VAL(OPT_NOCOLOR_INDEX)}
	},
	{
#define OPT_TERM CONF_TERM
#define OPT_TERM_INDEX (OPT_NOCOLOR_INDEX+1)
		.usage = " TERMNAME",
		.desc = {"set the TERM environment variable to TERMNAME.", 
				 "\tdefault: the current value of TERM in the environment.", NULL},
		.lopt = {OPT_TERM, 1, 0, LONG_ONLY_VAL(OPT_TERM_INDEX)}
	},
	{
#define OPT_NCTERM CONF_NCTERM
#define OPT_NCTERM_INDEX (OPT_TERM_INDEX+1)
		.usage = " TERMNAME",
		.desc = {"set the terminal profile used by ncurses.",
				 "\tdefault: the current value of TERM in the environment.", NULL},

		.lopt = {OPT_NCTERM, 1, 0, LONG_ONLY_VAL(OPT_NCTERM_INDEX)}
	},
	{
#define OPT_DEBUG CONF_DEBUG_FILE
#define OPT_DEBUG_INDEX (OPT_NCTERM_INDEX+1)
		.usage = " FILEPATH",
		.desc = {"collect debug messages into FILENAME.", NULL},
		.lopt = {OPT_DEBUG, 1, 0, 'd'}
	},
	{
#define OPT_PLUGIN_PATH CONF_PLUGIN_PATH
#define OPT_PLUGIN_PATH_INDEX (OPT_DEBUG_INDEX+1)
		.usage = " PATH[:PATH]...[:PATH]",
		.desc = {"a list of colon delimited directory paths to search for plugins.", 
					"\tdefault: " CONF_PLUGIN_PATH_DEFAULT, NULL},
		.lopt = {OPT_PLUGIN_PATH, 1, 0, LONG_ONLY_VAL(OPT_PLUGIN_PATH_INDEX)}
	},
	{
#define OPT_HELP "help"
#define OPT_HELP_INDEX (OPT_PLUGIN_PATH_INDEX+1)
		.usage = NULL,
		.desc = {"display this message.", NULL},
		.lopt = {OPT_HELP, 0, 0, 'h'}
	}
}; /* AUG_OPTIONS */
#define AUG_OPTLEN (OPT_HELP_INDEX+1)

static void init_long_options(struct option *long_options, char *optstring) {
	int i, os_i;

	os_i = 0;
	optstring[os_i++] = '+'; /* stop processing after first non-option */
	optstring[os_i++] = ':'; /* return : for missing arg */

	for(i = 0; i < AUG_OPTLEN; i++) {
		long_options[i] = AUG_OPTIONS[i].lopt;
		if(AUG_OPTIONS[i].lopt.val > 0 && AUG_OPTIONS[i].lopt.val < 256) {
			optstring[os_i++] = AUG_OPTIONS[i].lopt.val;
			switch(AUG_OPTIONS[i].lopt.has_arg) {
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


void opt_print_usage(FILE *file, int argc, const char *const argv[]) {
	fprintf(file, "usage: %s [OPTIONS] [CMD [ARG1, ...]]\n", (argc > 0)? argv[0] : "");
}

void opt_print_help(FILE *file, int argc, const char *const argv[]) {
	int i, k, f1_amt;
#define F1_SIZE 48
#define F1_WIDTH 44
	char f1[F1_SIZE];
	
	opt_print_usage(file, argc, argv);
	
	fprintf(file, "\n%-" AUG_EXPAND_QUOTE(F1_WIDTH) "s%s\n", "  CMD", "run CMD with arguments instead of the default");
	fprintf(file, "%-" AUG_EXPAND_QUOTE(F1_WIDTH) "s\tdefault: the value of SHELL in the environment\n", "");
	fprintf(file, "%-" AUG_EXPAND_QUOTE(F1_WIDTH) "s\tor if SHELL undefined: ", "");
	for(k = 0; k < CONF_DEFAULT_ARGC; k++)
		fprintf(file, "%s ", CONF_DEFAULT_ARGV[k]);
	
	fprintf(file, "\nOPTIONS:\n");
	for(i = 0; i < AUG_OPTLEN; i++) {
		f1_amt = 0;
		if(f1_amt < F1_SIZE && AUG_OPTIONS[i].lopt.val >= 0 && AUG_OPTIONS[i].lopt.val < 256) 
			f1_amt += snprintf(f1+f1_amt, F1_SIZE-f1_amt, "  -%c|", AUG_OPTIONS[i].lopt.val);
		else
			f1_amt += snprintf(f1+f1_amt, F1_SIZE-f1_amt, "  ");

		f1_amt += snprintf(f1+f1_amt, F1_SIZE-f1_amt, "--%s", AUG_OPTIONS[i].lopt.name);
		
		if(f1_amt < F1_SIZE && AUG_OPTIONS[i].lopt.has_arg && AUG_OPTIONS[i].usage != NULL)
			snprintf(f1+f1_amt, F1_SIZE-f1_amt, "%s", AUG_OPTIONS[i].usage);

		/*desc = AUG_OPTIONS[i].desc;
		do {
			fprintf(file, "%-" AUG_EXPAND_QUOTE(F1_WIDTH) "s%s\n", f1, *desc);
		} while( *(desc++) != NULL );*/
		for(k = 0; AUG_OPTIONS[i].desc[k] != NULL; k++) 
			fprintf(file, "%-" AUG_EXPAND_QUOTE(F1_WIDTH) "s%s\n", (k == 0? f1 : ""), AUG_OPTIONS[i].desc[k]);
	}
	
	fputc('\n', file);
}

static void print_char_reps() {
	uint32_t i;
	char s[8];

	for(i = 0; i <= 0x7f; i++) {
		if(screen_unctrl(i, s) != 0)
			err_exit(0, "could not derive unctrl string for 0x%02x", i);
		
		printf("%s", s);
#define AMT_PER_LINE 12
		if(i % AMT_PER_LINE == (AMT_PER_LINE - 1))
			printf("\n");
		else
			printf("\t");
	}

	printf("\n");
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
			
#define OPT_SET(_var, _value) conf_opt_set(conf, &_var); _var = _value
		switch (c) {
		case 'd': /* debug file */
			OPT_SET(conf->debug_file, optarg);
			break;
	
		case 'c': /* configuration file */
			OPT_SET(conf->conf_file, optarg);
			break;

		case 'e': /* configuration file */
			OPT_SET(conf->cmd_prefix, optarg);
			break;
	
		case 'E': /* configuration file */
			OPT_SET(conf->cmd_prefix_escape, optarg);
			break;

		case LONG_ONLY_VAL(OPT_NOCOLOR_INDEX):
			OPT_SET(conf->nocolor, true);
			break;

		case LONG_ONLY_VAL(OPT_TERM_INDEX):
			OPT_SET(conf->term, optarg);
			break;

		case LONG_ONLY_VAL(OPT_NCTERM_INDEX):
			OPT_SET(conf->ncterm, optarg);
			break;

		case LONG_ONLY_VAL(OPT_PLUGIN_PATH_INDEX):
			OPT_SET(conf->plugin_path, optarg);
			break;

#undef OPT_SET
		case 'h':
			errno = OPT_ERR_HELP;
			goto fail;
			break;
			
		case LONG_ONLY_VAL(OPT_CHAR_REP_INDEX):
			printf("character representations:\n");
			print_char_reps();
			errno = OPT_ERR_NONE;
			snprintf(opt_err_msg, 64, "\ne.g. aug -e ^A -E a\n");
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
