#include "aug_plugin.h"
#include "aug_api.h"

AUG_GLOBAL_API_OBJECTS;

const char aug_plugin_name[] = "reverse";

struct aug_plugin_cb g_callbacks = {
	.input_char = NULL,
	.screen_dims_change = NULL
};

static void reverse_coord(int size, int *coord) {
	*coord = (size-1) - *coord;
}

void cell_update(int rows, int cols, int *row, int *col, struct aug_cell *cell,
					aug_action *action, void *user) {
	(void)(rows);
	(void)(row);
	(void)(cell);
	(void)(action);
	(void)(user);

	reverse_coord(cols, col);
}

void cursor_move(int rows, int cols, int old_row, int old_col, 
		int *new_row, int *new_col, aug_action *action, void *user) {
	(void)(rows);
	(void)(old_row);
	(void)(old_col);
	(void)(new_row);
	(void)(action);
	(void)(user);

	reverse_coord(cols, new_col);
}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	AUG_API_INIT(plugin, api);

	aug_log("init\n");

	aug_callbacks_init(&g_callbacks);
	g_callbacks.cell_update = cell_update;
	g_callbacks.cursor_move = cursor_move;
	aug_callbacks(&g_callbacks, NULL);

	return 0;
}

void aug_plugin_free() {}

