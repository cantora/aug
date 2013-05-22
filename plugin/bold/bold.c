#include "aug_plugin.h"
#include "aug_api.h"

AUG_GLOBAL_API_OBJECTS;

const char aug_plugin_name[] = "bold";

void cell_update(
	int rows, int cols, int *row, int *col, wchar_t *wch, 
	attr_t *attr, int *color_pair, aug_action *action, void *user
);

struct aug_plugin_cb g_callbacks = {
	.input_char = NULL,
	.cell_update = cell_update,
	.cursor_move = NULL,
	.screen_dims_change = NULL
};

void cell_update(int rows, int cols, int *row, int *col, wchar_t *wch, 
		attr_t *attr, int *color_pair, aug_action *action, void *user) {
	(void)(rows);
	(void)(cols);
	(void)(row);
	(void)(col);
	(void)(wch);
	(void)(color_pair);
	(void)(action);
	(void)(user);
	
	*attr = *attr | A_BOLD;
}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	AUG_API_INIT(plugin, api);

	aug_log("init\n");

	g_callbacks.user = NULL;
	aug_callbacks(&g_callbacks, NULL);

	return 0;
}

void aug_plugin_free() {}

