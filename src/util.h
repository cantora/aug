#ifndef AUG_UTIL_H
#define AUG_UTIL_H

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define AUG_ARRAY_SIZE(_arr) ( sizeof(_arr)/sizeof(_arr[0]) )

static int set_nonblocking(int fd) {
	int opts;

	opts = fcntl(fd, F_GETFL);
	if (opts < 0) 
		return -1;

	opts = (opts | O_NONBLOCK);
	if (fcntl(fd, F_SETFL, opts) < 0)
		return -1;

	return 0;
}


static void change_winch(int how) {
	sigset_t set;

	if(sigemptyset(&set) != 0) 
		err_exit(errno, "sigemptyset failed"); 

	if(sigaddset(&set, SIGWINCH) != 0) 
		err_exit(errno, "sigaddset failed");

	if(sigprocmask(how, &set, NULL) != 0)
		err_exit(errno, "sigprocmask failed");
}

static inline void block_winch() {
	change_winch(SIG_BLOCK);
}

static inline void unblock_winch() {
	change_winch(SIG_UNBLOCK);
}

static void write_n_or_exit(int fd, const void *buf, size_t n) {
	ssize_t n_write, error;

	if( (n_write = write(fd, buf, n) ) != (ssize_t) n) {
		if(n_write > 0)
			error = 0;
		else
			error = errno;
		
		err_exit(error, "partial write");
	}
}


#endif