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
void aug_plugin_init(const struct aug_api_t *api, struct aug_plugin_t *plugin);
void aug_plugin_free(const struct aug_api_t *api, struct aug_plugin_t *plugin);

#endif /* AUG_AUG_PLUGIN_H */