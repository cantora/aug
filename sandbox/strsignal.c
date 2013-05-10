#define _POSIX_C_SOURCE 200809L 
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int sig;

	if(argc < 2) {
		printf("usage: %s N\n", argv[0]);
		return 1;
	}

	sig = strtol(argv[1], NULL, 10);
	printf("signal %d => %s\n", sig, strsignal(sig));
	return 0;
}