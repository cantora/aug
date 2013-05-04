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
#define NCT_USE_SCREEN
#include "api_test.h"

int main(int argc, char *argv[]) {
	FILE *fp;
	char *args[] = {
		argv[0], "-c", "./test/api_test_augrc", 
		"--plugin-path", 
		"./test/plugin/api_test:./test/plugin/fail_init:./test/plugin/unload:./plugin/hello", 
		"-d", "./build/log", NULL 
	};

	if(argc < 2) {
		printf("usage: %s FPATH\n", argv[0]);
		exit(1);
	}
	
	if( (fp = fopen(argv[1], "w")) == NULL) {
		printf("failed to open file at %s: %s\n", argv[1], strerror(errno));
		exit(1);
	}
	setvbuf(fp, NULL, _IONBF, 0);

	return api_test_main(fp, ARRAY_SIZE(args)-1, args);
}