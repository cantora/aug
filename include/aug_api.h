#ifndef AUG_AUG_API_H
#define AUG_AUG_API_H

#include "aug.h"

extern const struct aug_api *aug_api;
extern struct aug_plugin *aug_plugin;

#define AUG_API_HANDLE aug_api_handle
#define AUG_PLUGIN_HANDLE aug_plugin_handle
#include "aug_api_macros.h"

#define AUG_GLOBAL_API_VARIABLES \
	const struct aug_api *aug_api_handle; \
	struct aug_plugin *aug_plugin_handle;


#define AUG_API_INIT(plugin, api) \
	do { \
		aug_api_handle = api; \
		aug_plugin_handle = plugin; \
	} while(0)

#endif /* AUG_AUG_API_H */
