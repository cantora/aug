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

#include <stdlib.h>
#include <ccan/list/list.h>

#include "err.h"
#include "util.h"

struct edgewin_list {
	struct list_head head;
};

struct edgewin {
	int size;
	void *key;
	struct list_node node;
};

static struct {
	struct edgewin_list top_edgewins;
} g_map;

static void delete(struct edgewin *);

void region_map_init() {
	list_head_init(&g_map.top_edgewins.head);
}

void region_map_free() {
	struct edgewin *next, *i;

	list_for_each_safe(&g_map.top_edgewins.head, i, next, node) {
		delete(i);
	}
}

void region_map_push_top(void *key, int nlines) {
	struct edgewin *item;

	if(nlines < 1)
		err_exit(0, "nlines is less than 1");

	item = aug_malloc( sizeof(struct edgewin) );
	item->key = key;
	item->size = nlines;
	list_add_tail(&g_map.top_edgewins.head, &item->node);
}

static void delete(struct edgewin *item) {
	list_del(&item->node);
	free(item);
}

int region_map_delete(void *key) {
	struct edgewin *next, *i;

	list_for_each_safe(&g_map.top_edgewins.head, i, next, node) {
		if(i->key == key) {
			delete(i);
			return 0;
		}
	}

	return -1;
}

