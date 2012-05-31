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

#define AUG_PTR_NON_NULL( _ptr_expr ) \
	do { \
		if( ( _ptr_expr ) == NULL ) { \
			err_exit(0, "(" __FILE__ ":" stringify(__LINE__) ") " stringify( _ptr_expr ) \
							" == NULL"); \
		} \
	} while(0)


int set_nonblocking(int fd);
void write_n_or_exit(int fd, const void *buf, size_t n);


#endif