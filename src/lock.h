#ifndef AUG_LOCK_H
#define AUG_LOCK_H

#include <pthread.h>
#include <stdio.h>
#include "util.h"

#define AUG_LOCK_MEMBERS pthread_mutex_t aug_mtx

#define AUG_LOCK_INIT(_lockable_struct_ptr) \
	AUG_STATUS_EQUAL( pthread_mutex_init( &(_lockable_struct_ptr)->aug_mtx, NULL ), 0 )

#define AUG_LOCK_FREE(_lockable_struct_ptr) \
	AUG_STATUS_EQUAL( pthread_mutex_destroy( &(_lockable_struct_ptr)->aug_mtx ), 0 )

#define AUG_LOCK_DEBUG_PREFIX __FILE__ "@" stringify(__LINE__) 

static inline int fprint_tid(FILE *f, pthread_t pt) {
	size_t i,k;
	unsigned char *ptc;
	char buf[sizeof(pthread_t)*2 + 2 + 1];

	ptc = (unsigned char*)(void*)(&pt);
	i = 0;
	buf[i++] = '0';
	buf[i++] = 'x';
	for(k = 0; k < sizeof(pthread_t); k++) {
		snprintf(buf+i+k*2, 3, "%02x", (unsigned)(ptc[k]));
	}

	return fprintf(f, "%s", buf);
}

#define AUG_LOCK(_lockable_struct_ptr) \
	do { \
		fprint_tid(stderr, pthread_self()); \
		fprintf(stderr, AUG_LOCK_DEBUG_PREFIX ":lock " stringify(_lockable_struct_ptr) "\n"); \
		AUG_STATUS_EQUAL( pthread_mutex_lock( &(_lockable_struct_ptr)->aug_mtx ), 0 ); \
	} while(0)

#define AUG_UNLOCK(_lockable_struct_ptr) \
	do { \
		fprint_tid(stderr, pthread_self()); \
		fprintf(stderr, AUG_LOCK_DEBUG_PREFIX ":unlock " stringify(_lockable_struct_ptr) "\n" ); \
		AUG_STATUS_EQUAL( pthread_mutex_unlock( &(_lockable_struct_ptr)->aug_mtx ), 0 ); \
	} while(0)

#endif /* AUG_LOCK_H */