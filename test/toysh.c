#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "linenoise.h"

int main(int argc, char **argv) {
	char *line;
	int brk;
	(void)(argc);
	(void)(argv);

	printf("toysh starting up...\n");

	errno = 0;
	brk = 0;
	while((line = linenoise("[toysh]> ")) != NULL || errno == EAGAIN) {
		if(line == NULL) {
			printf("^C\n");
			errno = 0;
		}
		else {
			if(line[0] == '\0') {
				/* do nothing */
			}
			else if(strcmp(line, "exit") == 0) {
				brk = 1;
			}
			else if(strncmp(line, "echo ", 5) == 0) {
				puts(line+5);
			}
			else {
				printf("unknown command '%s'\n", line);
			}

			free(line);
		}

		if(brk != 0)
			break;
	}
	printf("toysh exiting...\n");
	fflush(stdout);
	sleep(1);

	return 0;
}