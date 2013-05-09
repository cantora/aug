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
#include "util.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>

#include "err.h"

int set_nonblocking(int fd) {
	int opts;

	opts = fcntl(fd, F_GETFL);
	if (opts < 0) 
		return -1;

	opts = (opts | O_NONBLOCK);
	if (fcntl(fd, F_SETFL, opts) < 0)
		return -1;

	return 0;
}

void write_n_or_exit(int fd, const void *buf, size_t n) {
	ssize_t n_write, error;

	if( (n_write = write(fd, buf, n) ) != (ssize_t) n) {
		if(n_write > 0)
			error = 0;
		else
			error = errno;

		err_exit(error, "partial write");
	}
}

inline void *aug_malloc(size_t size) {
	void *result = malloc(size);
	if(result == NULL)
		err_exit(0, "out of memory");

	return result;
}

int void_compare(const void *a, const void *b) {
	if(a < b) 
		return -1; 
	else if(a > b) 
		return 1;  
	else 
		return 0; 
}

void aug_detached_thread(void *(*fn)(void *), void *user, pthread_t *tid) {
	pthread_attr_t attr;
	int status;

	if( (status = pthread_attr_init(&attr)) != 0)
		err_exit(status, "failed to initialize pthread attr");

	status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(status != 0)
		err_exit(status, "failed to set detached thread attribute");

	if( (status = pthread_create(tid, &attr, fn, user)) != 0)
		err_exit(status, "failed to create unload thread");

	if( (status = pthread_attr_destroy(&attr)) != 0)
		err_warn(status, "failed to destroy pthread attr");
}
 

