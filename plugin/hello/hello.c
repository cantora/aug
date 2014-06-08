#include "aug_plugin.h"

const char aug_plugin_name[] = "hello";

int aug_plugin_start() {
	aug_log("hello world\n");

	return 0;
}

void aug_plugin_free() {
	aug_log("goodbye world\n");
}
