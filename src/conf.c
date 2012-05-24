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
