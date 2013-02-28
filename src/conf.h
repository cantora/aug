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

#define CONF_CMD_PREFIX "cmd-prefix"
#define CONF_CMD_PREFIX_DEFAULT "^A"

/* if no escape is defined, then aug will be in
 * 'pass-through' mode where if a command-prefix + 
 * following character are not a recognized command,
 * the command character and the following character
 * will simply pass through to the terminal.
 * this allows aug to use the same command character as a
 * screen/tmux session running inside aug with the appearance
 * of having simply 'rebound' certain key combinations to aug.
 * for example, suppose both aug and screen use command
 * character ^A, and screen is running inside of aug, and some
 * plugin command is bound to ^A-r, then typing ^A-r would of course
 * invoke the plugin command, typing ^A-^A would pass through
 * aug because it is unbound in the aug context (and thus invoke
 * the screen previous window command) and typing ^A-a would
 * pass through and cause screen to output a literal ^A to the 
 * terminal. 
 * if prefix escape is non-null, then aug will act like screen/tmux
 * where a command key followed by an unbound extension will simply
 * get filtered out and do nothing, and typing the command key
 * followed by the escape will output a literal command key to the
 * terminal.
 */
#define CONF_CMD_PREFIX_ESCAPE "cmd-prefix-escape"
#define CONF_CMD_PREFIX_ESCAPE_DEFAULT NULL

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
	const char *cmd_prefix;
	const char *cmd_prefix_escape;

	/* option (no config) */
	const char *conf_file;

	/* args */
	int cmd_argc;
	const char *const *cmd_argv;

	/* values derived from variables above */
	uint32_t cmd_key;
	uint32_t escape_key;

	/* objset to determine what was specified
	 * on the command line */
	struct aug_conf_opt_set opt_set;
};

extern const char *CONF_DEFAULT_ARGV[];
extern const int CONF_DEFAULT_ARGC;

void conf_init(struct aug_conf *conf);
void conf_opt_set(struct aug_conf *conf, void *addr);
int conf_opt_isset(const struct aug_conf *conf, void *addr);
void conf_merge_ini(struct aug_conf *conf, dictionary *ini);
int conf_set_derived_vars(struct aug_conf *conf, const char **err_msg);
void conf_fprint(struct aug_conf *c, FILE *f);


#endif /* AUG_CONF_H */