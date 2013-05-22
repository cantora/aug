#include "aug_plugin.h"
#include "aug_api.h"

const char aug_plugin_name[] = "hello";

AUG_GLOBAL_API_OBJECTS

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	AUG_API_INIT(plugin, api);

	aug_log("hello world\n");

	return 0;
}

void aug_plugin_free() {
	aug_log("goodbye world\n");
}
