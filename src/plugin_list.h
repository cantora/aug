#ifndef AUG_PLUGIN_LIST_H
#define AUG_PLUGIN_LIST_H

#include "aug.h"
#include <ccan/list/list.h>
#include "lock.h"

/* ccan list structures */
struct aug_plugin_list {
	struct list_head head;
	AUG_LOCK_MEMBERS;	
};

struct aug_plugin_item {
	struct aug_plugin plugin;	
	struct list_node node;
};


#define PLUGIN_LIST_FOREACH(_list_ptr, _item_ptr) \
	list_for_each( &(_list_ptr)->head, _item_ptr, node)

#define PLUGIN_LIST_FOREACH_REV(_list_ptr, _item_ptr) \
	list_for_each_rev( &(_list_ptr)->head, _item_ptr, node)

#define PLUGIN_LIST_FOREACH_SAFE(_list_ptr, _item_ptr, _next_ptr) \
	list_for_each_safe( &(_list_ptr)->head, _item_ptr, _next_ptr, node)

void plugin_list_init(struct aug_plugin_list *pl);
void plugin_list_free(struct aug_plugin_list *pl);
int plugin_list_push(struct aug_plugin_list *pl, const char *path, const char *name, 
						size_t namelen, const char **err_msg);
void plugin_list_del(struct aug_plugin_list *pl, struct aug_plugin_item *item);

#endif /* AUG_PLUGIN_LIST_H */