#ifndef AUG_UTIL_H
#define AUG_UTIL_H

#include <unistd.h>
#include <fcntl.h>

#include "err.h"

#define AUG_ARRAY_SIZE(_arr) ( sizeof(_arr)/sizeof(_arr[0]) )

int set_nonblocking(int fd);
void write_n_or_exit(int fd, const void *buf, size_t n);


#endif