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

#ifndef AUG_ERR_H
#define AUG_ERR_H

#include <stdio.h>

#define err_exit_call(error, ...) \
	err_exit_fn(error);

#define err_exit(...) \
	do { \
		err_log(stderr, __FILE__, __LINE__, __VA_ARGS__); \
		err_log(stdout, __FILE__, __LINE__, __VA_ARGS__); \
		err_exit_call(__VA_ARGS__); \
	} while(0)

#define err_warn(...) \
	err_log(stderr, __FILE__, __LINE__, __VA_ARGS__)

void err_exit_cleanup_fn(void (*cleanup_fn)(int error) );
void err_log(FILE *fp, const char *file, int lineno, 
					int error, const char *format, ...);
void err_exit_fn(int error);

#endif
