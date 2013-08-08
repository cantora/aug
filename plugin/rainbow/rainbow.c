#include <stdlib.h>
#include <time.h>

#include "aug_plugin.h"
#include "aug_api.h"

const char aug_plugin_name[] = "rainbow";

void cell_update(
	int rows, int cols, int *row, int *col, wchar_t *wch, 
	attr_t *attr, int *color_pair, aug_action *action, void *user
);

AUG_GLOBAL_API_OBJECTS;

struct aug_plugin_cb g_callbacks;

void cell_update(int rows, int cols, int *row, int *col, wchar_t *wch, 
		attr_t *attr, int *color_pair, aug_action *action, void *user) {
	(void)(rows);
	(void)(cols);
	(void)(row);
	(void)(col);
	(void)(wch);
	(void)(action);
	(void)(user);
	
	*color_pair = (rand() % 9)*9;
	*attr = *attr | A_BOLD;
}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	AUG_API_INIT(plugin, api);

	aug_log("init\n");

	aug_callbacks_init(&g_callbacks);
	g_callbacks.cell_update = cell_update;

	aug_callbacks(&g_callbacks, NULL);

	srand(time(NULL));
	return 0;
}

void aug_plugin_free() {}

