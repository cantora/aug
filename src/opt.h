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

#ifndef AUG_OPT_H
#define AUG_OPT_H

#include "conf.h"
#include <getopt.h>

enum opt_err {
	OPT_ERR_NONE = 0,
	OPT_ERR_UNKNOWN_OPTION,
	OPT_ERR_MISSING_ARG,
	OPT_ERR_USAGE, /* option requesting usage found */
	OPT_ERR_HELP /* option requesting help found */
};

extern char opt_err_msg[];
void opt_print_usage(FILE *file, int argc, const char *const argv[]);
void opt_print_help(FILE *file, int argc, const char *const argv[]);
int opt_parse(int argc, char *const argv[], struct aug_conf *conf);

#endif /* AUG_OPT_H */
