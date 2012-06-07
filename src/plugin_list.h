/* 
 * Copyright 2012 anthony cantor
 * This file is part of aug.
 *
 * aug is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * aug is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with aug.  If not, see <http://www.gnu.org/licenses/>.
 */
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