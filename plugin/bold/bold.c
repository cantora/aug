#include "aug_plugin.h"
#include "aug_api.h"

AUG_GLOBAL_API_OBJECTS;

const char aug_plugin_name[] = "bold";

struct aug_plugin_cb g_callbacks;

void cell_update(int rows, int cols, int *row, int *col, struct aug_cell *cell,
					aug_action *action, void *user) {
	(void)(rows);
	(void)(cols);
	(void)(row);
	(void)(col);
	(void)(action);
	(void)(user);

	cell->screen_attrs = cell->screen_attrs | A_BOLD;
}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	AUG_API_INIT(plugin, api);

	aug_log("init\n");

	aug_callbacks_init(&g_callbacks);
	g_callbacks.cell_update = cell_update;
	aug_callbacks(&g_callbacks, NULL);

	return 0;
}

void aug_plugin_free() {}

