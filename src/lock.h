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
#ifndef AUG_LOCK_H
#define AUG_LOCK_H

#include <pthread.h>
#include <stdio.h>
#include "util.h"

#ifdef AUG_LOCK_DEBUG
#	include <stdarg.h>
#	include "timer.h"
#endif

#ifdef AUG_LOCK_DEBUG
#	define AUG_LOCK_MEMBERS pthread_mutex_t aug_mtx; \
		int aug_lock_locked; \
		struct aug_timer aug_lock_tmr; \
		struct timeval aug_lock_elapsed;		
#else
#	define AUG_LOCK_MEMBERS pthread_mutex_t aug_mtx
#endif

#ifdef AUG_LOCK_DEBUG
#	define AUG_LOCK_INIT(_lockable_struct_ptr) \
		do { \
			AUG_STATUS_EQUAL( pthread_mutex_init( &(_lockable_struct_ptr)->aug_mtx, NULL ), 0 ); \
			(_lockable_struct_ptr)->aug_lock_locked = 0; \
		} while(0)
#else
#	define AUG_LOCK_INIT(_lockable_struct_ptr) \
		AUG_STATUS_EQUAL( pthread_mutex_init( &(_lockable_struct_ptr)->aug_mtx, NULL ), 0 )
#endif

#define AUG_LOCK_FREE(_lockable_struct_ptr) \
	AUG_STATUS_EQUAL( pthread_mutex_destroy( &(_lockable_struct_ptr)->aug_mtx ), 0 )

#ifdef AUG_LOCK_DEBUG
static inline int fprint_tid(const char *src, const char *func, int lineno, 
		FILE *f, pthread_t pt, const char *suffix, ...) {
#if defined(AUG_DEBUG) && defined(AUG_LOCK_DEBUG_PRINT)
	va_list args;
	size_t k;
#define FPRINT_TID_BUFLEN ( sizeof(pthread_t)*2 + 1 )
	unsigned char *ptc;
	char buf[FPRINT_TID_BUFLEN];
	char suf_buf[512];

	ptc = (unsigned char*)(void*)(&pt);
	for(k = 0; k < sizeof(pthread_t); k++) 
		snprintf(buf+k*2, 3, "%02x", (unsigned)(ptc[k]));

	buf[FPRINT_TID_BUFLEN-1] = '\0';
#undef FPRINT_TID_BUFLEN

	va_start(args, suffix);
	vsnprintf(suf_buf, sizeof(suf_buf), suffix, args);
	va_end(args);

	return fprintf(f, "(0x%s)%s#%s@%d=> %s\n", buf, src, func, lineno, suf_buf);
#else
	(void)(src);
	(void)(func);
	(void)lineno;
	(void)(f);
	(void)(pt);
	(void)(suffix);
	return 0;
#endif /* AUG_DEBUG && AUG_LOCK_DEBUG_PRINT */
}
#endif /* AUG_LOCK_DEBUG */

#ifdef AUG_LOCK_DEBUG
#define AUG_LOCK_CHECK_ELAPSED(_lockable_struct_ptr, _fmt) \
	do { \
		if( (_lockable_struct_ptr)->aug_lock_elapsed.tv_sec > 0 \
				|| (_lockable_struct_ptr)->aug_lock_elapsed.tv_usec > 100000) { \
			fprint_tid(__FILE__, __func__, __LINE__, stderr, pthread_self(), \
				_fmt, \
				(int) (_lockable_struct_ptr)->aug_lock_elapsed.tv_sec, \
				(int) (_lockable_struct_ptr)->aug_lock_elapsed.tv_usec \
			); \
		} \
	} while(0)

#endif

#ifdef AUG_LOCK_DEBUG
#	define AUG_LOCK(_lockable_struct_ptr) \
		do { \
			fprint_tid(__FILE__, __func__, __LINE__, stderr, pthread_self(), \
				":lock " stringify(_lockable_struct_ptr) ); \
			AUG_STATUS_EQUAL( timer_init( &(_lockable_struct_ptr)->aug_lock_tmr ), 0); \
			AUG_STATUS_EQUAL( pthread_mutex_lock( &(_lockable_struct_ptr)->aug_mtx ), 0 ); \
			AUG_STATUS_EQUAL( timer_elapsed( &(_lockable_struct_ptr)->aug_lock_tmr, \
				&(_lockable_struct_ptr)->aug_lock_elapsed ), 0); \
			AUG_LOCK_CHECK_ELAPSED( (_lockable_struct_ptr), \
				":waited %d secs, %d usecs for lock on " \
				stringify(_lockable_struct_ptr) ); \
			AUG_EQUAL( (_lockable_struct_ptr)->aug_lock_locked, 0 ); \
			(_lockable_struct_ptr)->aug_lock_locked = 1; \
			AUG_STATUS_EQUAL( timer_init( &(_lockable_struct_ptr)->aug_lock_tmr ), 0); \
		} while(0)
#else
#	define AUG_LOCK(_lockable_struct_ptr) \
		do { \
			AUG_STATUS_EQUAL( pthread_mutex_lock( &(_lockable_struct_ptr)->aug_mtx ), 0 ); \
		} while(0)
#endif

#ifdef AUG_LOCK_DEBUG
#	define AUG_UNLOCK(_lockable_struct_ptr) \
		do { \
			fprint_tid(__FILE__, __func__, __LINE__, stderr, pthread_self(), \
				":unlock " stringify(_lockable_struct_ptr) ); \
			AUG_EQUAL( (_lockable_struct_ptr)->aug_lock_locked, 1 ); \
			(_lockable_struct_ptr)->aug_lock_locked = 0; \
			AUG_STATUS_EQUAL( timer_elapsed( &(_lockable_struct_ptr)->aug_lock_tmr, \
				&(_lockable_struct_ptr)->aug_lock_elapsed ), 0); \
			AUG_LOCK_CHECK_ELAPSED( (_lockable_struct_ptr), \
				":held lock on " stringify(_lockable_struct_ptr) \
				" for %d secs, %d usecs" ); \
			AUG_STATUS_EQUAL( pthread_mutex_unlock( &(_lockable_struct_ptr)->aug_mtx ), 0 ); \
		} while(0)
#else
#	define AUG_UNLOCK(_lockable_struct_ptr) \
		do { \
			AUG_STATUS_EQUAL( pthread_mutex_unlock( &(_lockable_struct_ptr)->aug_mtx ), 0 ); \
		} while(0)
#endif

#endif /* AUG_LOCK_H */
