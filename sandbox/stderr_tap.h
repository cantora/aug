/* 
 * Copyright 2013 anthony cantor
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
#ifndef AUG_STDERR_TAP_H
#define AUG_STDERR_TAP_H

#include <stdio.h>

/* from ccan/str/str.h */
#define stringify(expr)		stringify_1(expr)
#define stringify_1(expr)	#expr

#ifdef diag
#	undef diag
#endif
#define diag(...) \
	do { \
		fputc('#', stderr); \
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
	} while(0)	

#ifdef ok1
#	undef ok1
#endif
#define ok1(e, ...) \
	do { \
		if(e) { \
			fprintf(stderr, "ok: "); \
		} \
		else { \
			fprintf(stderr, "failed: "); \
		} \
		fprintf(stderr, stringify(e) "\n"); \
	} while(0)	

#ifdef ok
#	undef ok
#endif
#define ok(e, ...) \
	do { \
		if(e) { \
			fprintf(stderr, "ok: "); \
		} \
		else { \
			fprintf(stderr, "failed: "); \
		} \
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
	} while(0)	

#ifdef pass
#	undef pass
#endif
#define pass(...) ok(1, __VA_ARGS__)

#ifdef fail
#	undef fail
#endif
#define fail(...) ok(0, __VA_ARGS__)

#ifdef plan_tests
#	undef plan_tests
#endif
#define plan_tests(n) (void)(n)

#ifdef todo_start
#	undef todo_start
#endif
#define todo_start(...) ok(0, __VA_ARGS__)

#ifdef todo_end
#	undef todo_end
#endif
#define todo_end() /* no-op */

#endif /* AUG_STDERR_TAP_H */