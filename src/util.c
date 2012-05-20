#include "util.h"
#include <errno.h>

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