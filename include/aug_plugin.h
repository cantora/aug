#ifndef AUG_AUG_PLUGIN_H
#define AUG_AUG_PLUGIN_H

#include "aug.h"

int aug_plugin_api_version_major() {
	return AUG_API_VERSION_MAJOR;
}

int aug_plugin_api_version_minor() {
	return AUG_API_VERSION_MINOR;
}

const char const aug_plugin_name[];
int aug_plugin_init( AUG_API_INIT_ARG_PROTO );
void aug_plugin_free( AUG_API_FREE_ARG_PROTO );

#endif /* AUG_AUG_PLUGIN_H */