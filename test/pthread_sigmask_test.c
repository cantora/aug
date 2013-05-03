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

#if defined(__APPLE__)
/* needed for SIGWINCH definition */
#	define _DARWIN_C_SOURCE 
#endif

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <ccan/tap/tap.h>

/* this is a portability test to make sure that child
 * threads inherity the signal mask of their parent
 */
#define err_exit(...) \
	do { \
		fprintf(stderr, __VA_ARGS__); \
		exit(1); \
	} while(0)

static void test_sigs_blocked() {
	sigset_t sigset;
	int winch_is_blocked, chld_is_blocked, s;

	diag("test that signals are blocked");
	if( (s = pthread_sigmask(SIG_SETMASK, NULL, &sigset)) != 0)
		err_exit("failed to get current sigset: %s\n", strerror(s));

	if( (winch_is_blocked = sigismember(&sigset, SIGWINCH) ) == -1 )
		err_exit("failed to test set membership\n");
	
	ok( (winch_is_blocked != 0), "confirm that SIGWINCH is blocked" );

	if( (chld_is_blocked = sigismember(&sigset, SIGCHLD) ) == -1 )
		err_exit("failed to test set membership\n");

	ok( (chld_is_blocked != 0), "confirm that SIGCHLD is blocked" );
}

static void test_sigs_unblocked() {
	sigset_t sigset;
	int winch_is_blocked, chld_is_blocked, s;

	diag("test that signals are unblocked");
	if( (s = pthread_sigmask(SIG_SETMASK, NULL, &sigset)) != 0)
		err_exit("failed to get current sigset: %s\n", strerror(s));

	if( (winch_is_blocked = sigismember(&sigset, SIGWINCH) ) == -1 )
		err_exit("failed to test set membership\n");
	
	ok( (winch_is_blocked == 0), "confirm that SIGWINCH is unblocked" );

	if( (chld_is_blocked = sigismember(&sigset, SIGCHLD) ) == -1 )
		err_exit("failed to test set membership\n");
	
	ok( (chld_is_blocked == 0), "confirm that SIGCHLD is unblocked" );
}

static void *thread_fn(void *arg) {
	(void)(arg);

	test_sigs_blocked();
	return NULL;	
}

int main(int argc, char *argv[]) {
	pthread_t tid;
	sigset_t set, oset;
	int s;
	(void)(argc);
	(void)(argv);

	plan_tests(2 + 2 + 2 + 2 + 2);
	test_sigs_unblocked();

	sigemptyset(&set);
	sigaddset(&set, SIGWINCH);
	sigaddset(&set, SIGCHLD);

	if( (s = pthread_sigmask(SIG_BLOCK, &set, &oset)) != 0)
		err_exit("failed to block signals: %s\n", strerror(s) );
	test_sigs_blocked();
	
	if( (s = pthread_create(&tid, NULL, &thread_fn, NULL)) != 0)
		err_exit("failed to create thread: %s\n", strerror(s));

	test_sigs_blocked();
	if( (s = pthread_sigmask(SIG_SETMASK, &oset, NULL)) != 0)
		err_exit("failed to unblock signals: %s\n", strerror(s) );
	test_sigs_unblocked();

	if( (s = pthread_join(tid, NULL)) != 0)
		err_exit("failed to join thread: %s\n", strerror(s));
	
	return exit_status();
}
