#ifndef AUG_AUG_PLUGIN_H
#define AUG_AUG_PLUGIN_H

#include "aug.h"

void aug_plugin_init(const struct aug_api_t *api);
void aug_plugin_free(const struct aug_api_t *api);

#endif /* AUG_AUG_PLUGIN_H */