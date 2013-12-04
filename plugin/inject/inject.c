#include "aug_plugin.h"
#include "aug_api.h"

#include <stdlib.h>

const char aug_plugin_name[] = "inject";

AUG_GLOBAL_API_OBJECTS

struct aug_plugin_cb g_callbacks;

#define INJECT_HLEN 1024 /* remember a maximum of 1024 characters */
static uint32_t g_history[INJECT_HLEN]; 
static size_t g_hend;

void input_char(uint32_t *ch, aug_action *action, struct aug_inject *inject, void *user) {
	(void)(user);

	aug_log("inject: %x\n", *ch);
	if(*ch == 0x16) { /* ^V */
		*action = AUG_ACT_CANCEL;
		inject->chars = g_history;
		inject->len = g_hend;
		g_hend = 0;
	}
	else {
		if(g_hend < INJECT_HLEN)
			g_history[g_hend++] = *ch;
	}

}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	AUG_API_INIT(plugin, api);

	aug_log("inject\n");

	g_hend = 0;

	aug_callbacks_init(&g_callbacks);
	g_callbacks.input_char = input_char;
	aug_callbacks(&g_callbacks, NULL);

	return 0;
}

void aug_plugin_free() {}
