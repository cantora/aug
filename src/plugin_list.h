#ifndef AUG_PLUGIN_LIST_H
#define AUG_PLUGIN_LIST_H

#include <ccan/list/list.h>

/* ccan list structures */
struct aug_plugin_list {
	struct list_head head;
};

void plugin_list_init(struct aug_plugin_list *pl);
void plugin_list_free(struct aug_plugin_list *pl);
int plugin_list_push(struct aug_plugin_list *pl, const char *path, 
						const char **err_msg);
#endif /* AUG_PLUGIN_LIST_H */