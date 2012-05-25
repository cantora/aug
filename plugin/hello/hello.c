#include "aug_plugin.h"

const char *const aug_plugin_name = "hello";

void aug_plugin_init(const struct aug_api *api, struct aug_plugin *plugin) {
	(void)(api);
	(void)(plugin);
}

void aug_plugin_free(const struct aug_api *api, struct aug_plugin *plugin) {
	(void)(api);
	(void)(plugin);
}