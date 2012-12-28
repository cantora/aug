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

#define AUG_LOCK_DEBUG

#ifdef AUG_LOCK_DEBUG
#	define AUG_LOCK_MEMBERS pthread_mutex_t aug_mtx; \
		int locked;
#else
#	define AUG_LOCK_MEMBERS pthread_mutex_t aug_mtx
#endif

#ifdef AUG_LOCK_DEBUG
#	define AUG_LOCK_INIT(_lockable_struct_ptr) \
		do { \
			AUG_STATUS_EQUAL( pthread_mutex_init( &(_lockable_struct_ptr)->aug_mtx, NULL ), 0 ); \
			(_lockable_struct_ptr)->locked = 0; \
		} while(0)
#else
#	define AUG_LOCK_INIT(_lockable_struct_ptr) \
		AUG_STATUS_EQUAL( pthread_mutex_init( &(_lockable_struct_ptr)->aug_mtx, NULL ), 0 )
#endif

#define AUG_LOCK_FREE(_lockable_struct_ptr) \
	AUG_STATUS_EQUAL( pthread_mutex_destroy( &(_lockable_struct_ptr)->aug_mtx ), 0 )

#define AUG_LOCK_DEBUG_PREFIX __FILE__ "@" stringify(__LINE__) 

#ifdef AUG_LOCK_DEBUG
static inline int fprint_tid(FILE *f, pthread_t pt, const char *suffix) {
#ifdef AUG_DEBUG
	size_t k;
#define FPRINT_TID_BUFLEN ( sizeof(pthread_t)*2 + 1 )
	unsigned char *ptc;
	char buf[FPRINT_TID_BUFLEN];
	
	ptc = (unsigned char*)(void*)(&pt);
	for(k = 0; k < sizeof(pthread_t); k++) 
		snprintf(buf+k*2, 3, "%02x", (unsigned)(ptc[k]));

	buf[FPRINT_TID_BUFLEN-1] = '\0';
#undef FPRINT_TID_BUFLEN

	return fprintf(f, "THREAD(0x%s)=> %s\n", buf, suffix);
#else
	return 0;
#endif
}
#endif

#ifdef AUG_LOCK_DEBUG
#	define AUG_LOCK(_lockable_struct_ptr) \
		do { \
			fprint_tid(stderr, pthread_self(), AUG_LOCK_DEBUG_PREFIX ":lock " stringify(_lockable_struct_ptr) ); \
			AUG_STATUS_EQUAL( pthread_mutex_lock( &(_lockable_struct_ptr)->aug_mtx ), 0 ); \
			AUG_EQUAL( (_lockable_struct_ptr)->locked, 0 ); \
			(_lockable_struct_ptr)->locked = 1; \
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
			fprint_tid(stderr, pthread_self(), AUG_LOCK_DEBUG_PREFIX ":unlock " stringify(_lockable_struct_ptr) ); \
			AUG_EQUAL( (_lockable_struct_ptr)->locked, 1 ); \
			(_lockable_struct_ptr)->locked = 0; \
			AUG_STATUS_EQUAL( pthread_mutex_unlock( &(_lockable_struct_ptr)->aug_mtx ), 0 ); \
		} while(0)
#else
#	define AUG_UNLOCK(_lockable_struct_ptr) \
		do { \
			AUG_STATUS_EQUAL( pthread_mutex_unlock( &(_lockable_struct_ptr)->aug_mtx ), 0 ); \
		} while(0)
#endif

#endif /* AUG_LOCK_H */