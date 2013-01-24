#include "api_test.h"

int main(int argc, char *argv[]) {
	char *args[] = {
		argv[0], "-c", "./test/api_test_augrc", 
		"--plugin-path", 
		"./test/plugin/api_test:./test/plugin/fail_init:./test/plugin/unload:./plugin/hello", 
		"-d", "./build/log", NULL 
	};
	(void)(argc);
	
	return api_test_main(ARRAY_SIZE(args)-1, args);
}