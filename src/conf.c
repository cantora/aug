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

#undef MERGE_VAR
}


void conf_plugin_path_next(struct aug_conf *conf, char **current, size_t *size, char **next) {
	char delim = ':';
	
	if(*current == NULL)
		*current = conf->plugin_path;

	*next = strchr(*current, delim);
	if(next == NULL)
		*size = strlen(*current);
	else
		*size = *next - *current;
}
void conf_plugin_path_dirs(struct aug_conf *conf, darray(char *) *dirs) {
	char *plugin_path, *delim;

	delim = ":";
	plugin_path = strdup(conf->plugin_path);
	if(plugin_path == NULL)
		err_exit(errno, "memory error!");

	for(itr = strtok(plugin_path, delim); itr != NULL; itr = strtok(NULL, delim) ) {
		darray_append()
	}

	free(plugin_path);
}