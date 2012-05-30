#ifndef AUG_LOCK_H
#define AUG_LOCK_H

#include <pthread.h>
#include "util.h"

#define AUG_LOCK_MEMBERS pthread_mutex_t aug_mtx

#define AUG_LOCK_INIT(_lockable_struct_ptr) \
	AUG_STATUS_EQUAL( pthread_mutex_init( &(_lockable_struct_ptr)->aug_mtx, NULL ), 0 )

#define AUG_LOCK_FREE(_lockable_struct_ptr) \
	AUG_STATUS_EQUAL( pthread_mutex_destroy( &(_lockable_struct_ptr)->aug_mtx ), 0 )
	
#define AUG_LOCK(_lockable_struct_ptr) \
	AUG_STATUS_EQUAL( pthread_mutex_lock( &(_lockable_struct_ptr)->aug_mtx ), 0 )

#define AUG_UNLOCK(_lockable_struct_ptr) \
	AUG_STATUS_EQUAL( pthread_mutex_unlock( &(_lockable_struct_ptr)->aug_mtx ), 0 )

#endif /* AUG_LOCK_H */