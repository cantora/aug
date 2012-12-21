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

#include "region_map.h"

#include <stdlib.h>
#include <ccan/list/list.h>

#include "err.h"
#include "util.h"

struct edgewin_list {
	struct list_head head;
};

struct edgewin {
	int size;
	const void *key;
	struct list_node node;
};

static struct {
	struct edgewin_list top_edgewins;
} g_map;

static void delete(struct edgewin *);
static void init_region(int, int, int, int, int, struct aug_region *);

void region_map_init() {
	list_head_init(&g_map.top_edgewins.head);
}

void region_map_free() {
	struct edgewin *next, *i;

	list_for_each_safe(&g_map.top_edgewins.head, i, next, node) {
		delete(i);
	}
}

void region_map_push_top(const void *key, int nlines) {
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

int region_map_delete(const void *key) {
	struct edgewin *next, *i;

	list_for_each_safe(&g_map.top_edgewins.head, i, next, node) {
		if(i->key == key) {
			delete(i);
			return 0;
		}
	}

	return -1;
}

static int void_compare(const void *a, const void *b) {
	if(a < b) 
		return -1;
	else if(a > b)
		return 1;
	else
		return 0;
}

AVL *region_map_key_dims_alloc() {
	return avl_new( (AvlCompare) void_compare );
}

void region_map_key_dims_clear(AVL *key_dims) {
	AvlIter i;
	struct aug_region *region;

	avl_foreach(i, key_dims) {
		region = i.value;
		free(region);
	}
}

void region_map_key_dims_free(AVL *key_dims) {
	region_map_key_dims_clear(key_dims);
	free(key_dims);
}

int region_map_dims(int lines, int columns, AVL *key_dims, struct aug_region *primary) {
	int rows, cols, tmp;
	struct edgewin *i;
	struct aug_region *region;

	rows = lines;
	cols = columns;

	if(rows < 1 || cols < 1)
		return -1;

	list_for_each(&g_map.top_edgewins.head, i, node) {
		region = malloc( sizeof( struct aug_region ) );
		if(region == NULL)
			err_exit(0, "out of memory");

		tmp = (lines-rows);
		rows -= i->size;
		init_region(rows, cols, tmp, 0, i->size, region);
		avl_insert(key_dims, i->key, region);
	}

	init_region(rows, cols, lines-rows, 0, rows, primary);
	return 0;
}

static void init_region(int rows, int cols, int y, int x, int size, struct aug_region *region) {
	if(rows < 1) {
		region->rows = 0;
		region->cols = 0;
	}
	else {
		region->rows = size; 
		region->cols = cols;
		region->y = y;
		region->x = x;
	}
}
