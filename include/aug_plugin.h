#ifndef AUG_AUG_PLUGIN_H
#define AUG_AUG_PLUGIN_H

#include "aug.h"

int aug_plugin_api_version_major() {
	return AUG_API_VERSION_MAJOR;
}

int aug_plugin_api_version_minor() {
	return AUG_API_VERSION_MINOR;
}

const char *const aug_plugin_name;
void aug_plugin_init(const struct aug_api *api, struct aug_plugin *plugin);
void aug_plugin_free(const struct aug_api *api, struct aug_plugin *plugin);

#endif /* AUG_AUG_PLUGIN_H */