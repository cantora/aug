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
#ifndef AUG_UTIL_H
#define AUG_UTIL_H

#include <unistd.h>
#include <fcntl.h>
#include <ccan/str/str.h>
#include "err.h"

#define AUG_ARRAY_SIZE(_arr) ( sizeof(_arr)/sizeof(_arr[0]) )

#define AUG_STATUS_EQUAL(_func_call, _status) \
	do { \
		int status; \
		if( ( status = _func_call ) != ( _status ) ) { \
			err_exit(0, "(" __FILE__ ":" stringify(__LINE__) ") " stringify(_func_call) \
							" = %d != " stringify(_status), status); \
		} \
	} while(0)

#define AUG_EQUAL(_var, _val) \
	do { \
		if( ( _var ) != ( _val ) ) { \
			err_exit(0, "(" __FILE__ ":" stringify(__LINE__) ") " stringify(_var) \
							" = %d != " stringify(_val), _var); \
		} \
	} while(0)

#define AUG_PTR_NON_NULL( _ptr_expr ) \
	do { \
		if( ( _ptr_expr ) == NULL ) { \
			err_exit(0, "(" __FILE__ ":" stringify(__LINE__) ") " stringify( _ptr_expr ) \
							" == NULL"); \
		} \
	} while(0)


int set_nonblocking(int fd);
void write_n_or_exit(int fd, const void *buf, size_t n);
void *aug_malloc(size_t size);
int void_compare(const void *a, const void *b);

#endif