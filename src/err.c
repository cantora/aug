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

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static void (*err_exit_cleanup)(int error) = NULL;

void err_exit_cleanup_fn(void (*cleanup_fn)(int error) ) {
	err_exit_cleanup = cleanup_fn;
}

void err_log(FILE *fp, const char *file, int lineno, 
					int error, const char *format, ...) {
	va_list args;
	size_t buflen = 1024;
	char buf[buflen];
	int result;

	va_start(args, format);
	result = vsnprintf(buf, buflen, format, args);
	va_end(args);

	if(result >= 0) {
		fprintf(fp, "(%s:%d) ", file, lineno);
		fwrite(buf, sizeof(char), result, fp);
		if(error != 0) 
			fprintf(fp, ": %s (%d)", strerror(error), error );
		
		fputc('\n', fp);
		fflush(fp);
	}
}

void err_exit_fn(int error) {
	if(err_exit_cleanup != NULL)
		(*err_exit_cleanup)(error);

#ifdef AUG_ERR_COREDUMP
	*( (int *) 0) = 1;
#endif
	exit(1);
}