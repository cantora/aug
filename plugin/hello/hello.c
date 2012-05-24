#include "aug_plugin.h"

const char *const aug_plugin_name = "hello";

void aug_plugin_init(const struct aug_api_t *api, struct aug_plugin_t *plugin) {
	(void)(api);
	(void)(plugin);
}

void aug_plugin_free(const struct aug_api_t *api, struct aug_plugin_t *plugin) {
	(void)(api);
	(void)(plugin);
}