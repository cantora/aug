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
#include "conf.h"
#include <stdlib.h>
#include <assert.h>
#include "screen.h"
#include "err.h"

const char *CONF_DEFAULT_ARGV[] = AUG_DEFAULT_ARGV;
const int CONF_DEFAULT_ARGC = (sizeof(CONF_DEFAULT_ARGV)/sizeof(char *)) - 1;

void conf_init(struct aug_conf *conf) {
	const char *shell;

	objset_init(&conf->opt_set);

	conf->term = CONF_TERM_DEFAULT;
	conf->ncterm = CONF_NCTERM_DEFAULT;
	conf->debug_file = CONF_DEBUG_FILE_DEFAULT;
	conf->nocolor = !CONF_COLOR_DEFAULT;
	conf->conf_file = CONF_CONFIG_FILE_DEFAULT;
	conf->plugin_path = CONF_PLUGIN_PATH_DEFAULT;
	conf->cmd_prefix = CONF_CMD_PREFIX_DEFAULT;
	conf->cmd_prefix_escape = CONF_CMD_PREFIX_ESCAPE_DEFAULT;

	shell = getenv("SHELL");
	if(shell != NULL) {
		CONF_DEFAULT_ARGV[0] = shell;
		CONF_DEFAULT_ARGV[1] = NULL;
		conf->cmd_argc = 1;
	}
	else {
		conf->cmd_argc = CONF_DEFAULT_ARGC;
	}
	conf->cmd_argv = CONF_DEFAULT_ARGV;
}

void conf_opt_set(struct aug_conf *conf, void *addr) {
	assert(addr > (void *)conf);
	assert(addr < (void *) (conf + sizeof(struct aug_conf) ) );
	
	objset_add(&conf->opt_set, addr);
}

int conf_opt_isset(const struct aug_conf *conf, void *addr) {
	assert(addr > (void *)conf);
	assert(addr < (void *) (conf + sizeof(struct aug_conf) ) );
	
	return (objset_get(&conf->opt_set, addr) != NULL);
}

void conf_merge_ini(struct aug_conf *conf, dictionary *ini) {

	/* we keep track of what was specified on the command
	 * line with conf->opt_set because the command line
	 * takes precedence over the ini file which takes
	 * precedence over the hardcoded defaults. thus we dont
	 * even look in the ini file for a value if that value
	 * was set from the command line */
#	define MERGE_VAR(_var, _type, _name, _default) \
		if(objset_get(&conf->opt_set, &conf->_var) == 0) { \
			conf->_var = ciniparser_get##_type( \
				ini, CONF_CONFIG_SECTION_CORE ":" _name, \
				_default \
			); \
		}
	/* MERGE_VAR */

	MERGE_VAR(nocolor, boolean, CONF_COLOR, !CONF_COLOR_DEFAULT)
	MERGE_VAR(term, string, CONF_TERM, CONF_TERM_DEFAULT)
	MERGE_VAR(ncterm, string, CONF_NCTERM, CONF_NCTERM_DEFAULT)
	MERGE_VAR(debug_file, string, CONF_DEBUG_FILE, CONF_DEBUG_FILE_DEFAULT)
	MERGE_VAR(plugin_path, string, CONF_PLUGIN_PATH, CONF_PLUGIN_PATH_DEFAULT)
	MERGE_VAR(cmd_prefix, string, CONF_CMD_PREFIX, CONF_CMD_PREFIX_DEFAULT)
	MERGE_VAR(cmd_prefix_escape, string, CONF_CMD_PREFIX_ESCAPE, CONF_CMD_PREFIX_ESCAPE_DEFAULT)

#undef MERGE_VAR
}

static int char_rep_to_char(const char *str, uint32_t *ch) {
	uint32_t i;
	char s[8];

	for(i = 0; i <= 0xff; i++) {
		if(screen_unctrl(i, s) != 0)
			err_exit(0, "could not derive unctrl string for 0x%02x", i);

		if(strcasecmp(str, s) == 0) {
			*ch = i;
			break;	
		}
	}

	if(i > 0xff)
		return -1;

	return 0;
}

int conf_set_derived_vars(struct aug_conf *conf, const char **err_msg) {
	if( char_rep_to_char(conf->cmd_prefix, &conf->cmd_key) != 0) {
		*err_msg = "could not understand command prefix character representation.";
		return -1;
	}

	if(conf->cmd_prefix_escape != NULL) {
		if( char_rep_to_char(conf->cmd_prefix_escape, &conf->escape_key) != 0) {
			*err_msg = "could not understand command prefix character representation.";
			return -1;
		}
	}
	else 
		conf->escape_key = -1;

	return 0;	
}

void conf_fprint(struct aug_conf *c, FILE *f) {
	int i;

	fprintf(f, "nocolor: \t\t'%d'\n", c->nocolor);
	fprintf(f, "ncterm: \t\t'%s'\n", c->ncterm);
	fprintf(f, "term: \t\t\t'%s'\n", c->term);
	fprintf(f, "debug_file: \t\t'%s'\n", c->debug_file);
	fprintf(f, "conf_file: \t\t'%s'\n", c->conf_file);
	fprintf(f, "cmd_prefix: \t\t'%s'\n", c->cmd_prefix);
	fprintf(f, "cmd_prefix_escape: \t'%s'\n", c->cmd_prefix_escape);
	fprintf(f, "cmd: \t\t\t[");
	for(i = 0; i < c->cmd_argc; i++) {
		fprintf(f, "'%s'", c->cmd_argv[i]);
		if(i < c->cmd_argc - 1)
			fprintf(f, ", ");
	}
	fprintf(f, "]\n");

	fprintf(f, "cmd_key: 0x%02x\n", c->cmd_key);
	if(c->escape_key >= 0)
		fprintf(f, "escape_key: 0x%02x\n", c->escape_key);
	else
		fprintf(f, "pass through mode = on\n");
}
