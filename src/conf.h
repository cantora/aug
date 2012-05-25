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
#ifndef AUG_CONF_H
#define AUG_CONF_H

#include <stdbool.h>
#include <ccan/objset/objset.h>
#include <ccan/ciniparser/ciniparser.h>
#include <ccan/darray/darray.h>

#define CONF_CONFIG_FILE_DEFAULT "~/.augrc"
#define CONF_CONFIG_SECTION_CORE "aug"

/* when adding a new config variable:
 * 		1. create a CONF_* and CONF_*_DEFAULT variable
 *		2. add an element to the aug_conf struct
 *		3. add a line to conf_init to initialize the element
 *			in aug_conf
 * 		4. add a MERGE_VAR line to conf_merge_ini
 * 		5. if desired, add necessary data to opt.c (following instructions there)
 */
#define CONF_COLOR "color"
#define CONF_COLOR_DEFAULT true

#define CONF_TERM "term"
#define CONF_TERM_DEFAULT NULL /* use the value of TERM in current environment */

#define CONF_NCTERM "ncterm"
#define CONF_NCTERM_DEFAULT NULL /* use current TERM environment variable */

#define CONF_DEBUG_FILE "debug"
#define CONF_DEBUG_FILE_DEFAULT NULL /* redirect stderr to /dev/null */

#define CONF_PLUGIN_PATH "plugin-path"
#define CONF_PLUGIN_PATH_DEFAULT "~/.aug-plugins"

struct aug_conf_opt_set {
	OBJSET_MEMBERS(const void *);
};

struct aug_conf {
	/* options/config variables */
	bool nocolor;
	const char *ncterm;
	const char *term;
	const char *debug_file;
	const char *plugin_path;

	/* option (no config) */
	const char *conf_file;

	/* args */
	int cmd_argc;
	const char *const *cmd_argv;

	/* objset to determine what was specified
	 * on the command line */
	struct aug_conf_opt_set opt_set;
};

extern const char *CONF_DEFAULT_ARGV[];
extern const int CONF_DEFAULT_ARGC;

void conf_init(struct aug_conf *conf);
void conf_opt_set(struct aug_conf *conf, void *addr);
void conf_merge_ini(struct aug_conf *conf, dictionary *ini);

#endif /* AUG_CONF_H */