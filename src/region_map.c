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

struct edgewin {
	int size;
	const void *key;
	struct list_node node;
};

static struct {
	struct list_head top_edgewins;
	struct list_head bot_edgewins;
	struct list_head left_edgewins;
	struct list_head right_edgewins;
} g_map;

static void delete(struct edgewin *);
static int init_region(int, int, int, int, int, int, struct aug_region *);

void region_map_init() {
	list_head_init(&g_map.top_edgewins);
	list_head_init(&g_map.bot_edgewins);
	list_head_init(&g_map.left_edgewins);
	list_head_init(&g_map.right_edgewins);
}

void region_map_free() {
	struct edgewin *next, *i;

	list_for_each_safe(&g_map.top_edgewins, i, next, node) {
		delete(i);
	}
	list_for_each_safe(&g_map.bot_edgewins, i, next, node) {
		delete(i);
	}
	list_for_each_safe(&g_map.left_edgewins, i, next, node) {
		delete(i);
	}
	list_for_each_safe(&g_map.right_edgewins, i, next, node) {
		delete(i);
	}
}

static inline void edgewin_list_push(struct list_head *head, const void *key, int size) {
	struct edgewin *item;

	if(size < 1)
		err_exit(0, "size is less than 1: %d", size);

	item = aug_malloc( sizeof(struct edgewin) );
	item->key = key;
	item->size = size;
	list_add_tail(head, &item->node);
}

void region_map_push_top(const void *key, int nlines) { 
	edgewin_list_push(&g_map.top_edgewins, key, nlines);
}
void region_map_push_bot(const void *key, int nlines) { 
	edgewin_list_push(&g_map.bot_edgewins, key, nlines);
}
void region_map_push_left(const void *key, int ncols) { 
	edgewin_list_push(&g_map.left_edgewins, key, ncols);
}
void region_map_push_right(const void *key, int ncols) { 
	edgewin_list_push(&g_map.right_edgewins, key, ncols);
}

static void delete(struct edgewin *item) {
	list_del(&item->node);
	free(item);
}

static int edgewin_list_size(const struct list_head *head) {
	struct edgewin *i;
	int n;

	n = 0;
	list_for_each(head, i, node) 
		n++;

	return n;
}

int region_map_top_size() { return edgewin_list_size(&g_map.top_edgewins); }
int region_map_bot_size() { return edgewin_list_size(&g_map.bot_edgewins); }
int region_map_left_size() { return edgewin_list_size(&g_map.left_edgewins); }
int region_map_right_size() { return edgewin_list_size(&g_map.right_edgewins); }

int region_map_delete(const void *key) {
	struct edgewin *next, *i;
	int status = -1;

	list_for_each_safe(&g_map.top_edgewins, i, next, node) {
		if(i->key == key) {
			delete(i);
			status = 0;
		}
	}
	list_for_each_safe(&g_map.bot_edgewins, i, next, node) {
		if(i->key == key) {
			delete(i);
			status = 0;
		}
	}
	list_for_each_safe(&g_map.left_edgewins, i, next, node) {
		if(i->key == key) {
			delete(i);
			status = 0;
		}
	}
	list_for_each_safe(&g_map.right_edgewins, i, next, node) {
		if(i->key == key) {
			delete(i);
			status = 0;
		}
	}

	return status;
}

AVL *region_map_key_regs_alloc() {
	return avl_new( (AvlCompare) void_compare );
}

void region_map_key_regs_clear(AVL **key_regs) {
	region_map_key_regs_free(*key_regs);
	*key_regs = region_map_key_regs_alloc();
}

void region_map_key_regs_free(AVL *key_regs) {
	AvlIter i;
	
	avl_foreach(i, key_regs) {
		free(i.value);
	}
	avl_free(key_regs);
}

static void apply_horizontal_edgewins(struct list_head *edgewins, AVL *key_regs,
			int lines, int *rows_left, int cols_left, int reverse) {
	struct edgewin *i;
	struct aug_region *region;
	int y, allocd;

	y = lines;
	list_for_each(edgewins, i, node) {
		allocd = 0;
		if( (region = avl_lookup(key_regs, i->key)) == NULL) {
			region = malloc( sizeof( struct aug_region ) );
			if(region == NULL)
				err_exit(0, "out of memory");

			allocd = 1;
		}
	
		if(reverse == 0)
			y = lines - *rows_left;
		else {
			y = y - (i->size);
		}
	
		if(init_region(*rows_left-(i->size), cols_left, 
				y, 0, i->size, cols_left, region) == 0)
			*rows_left -= i->size;

		if(allocd != 0)
			avl_insert(key_regs, i->key, region);
	}
}

static void apply_vertical_edgewins(struct list_head *edgewins, AVL *key_regs,
			int start_y, int cols, int rows_left, int *cols_left, int reverse) {
	struct edgewin *i;
	struct aug_region *region;
	int x;

	x = cols;
	list_for_each(edgewins, i, node) {
		region = malloc( sizeof( struct aug_region ) );
		if(region == NULL)
			err_exit(0, "out of memory");
		
		if(reverse == 0)			
			x = cols - *cols_left;
		else
			x = x - (i->size);

		if(init_region(rows_left, *cols_left - (i->size), \
				start_y, x, rows_left, i->size, region) == 0)
			*cols_left -= i->size;
		avl_insert(key_regs, i->key, region);
	}
}

/* apply the region map to a rectangle of lines X columns dimension
 * and store the result in key_regs which is a map from keys to regions.
 * the leftover space is described by the primary output parameter.
 */
int region_map_apply(int lines, int columns, AVL *key_regs, struct aug_region *primary) {
	int rows, cols, primary_y, primary_x;

	rows = lines;
	cols = columns;

	if(rows < 1 || cols < 1)
		return -1;

	apply_horizontal_edgewins(
		&g_map.top_edgewins, 
		key_regs,
		lines, 
		&rows,
		cols,
		0
	);
	primary_y = lines-rows;

	apply_horizontal_edgewins(
		&g_map.bot_edgewins, 
		key_regs,
		lines, 
		&rows,
		cols,
		1
	);

	apply_vertical_edgewins(
		&g_map.left_edgewins, 
		key_regs,
		primary_y,
		columns, 
		rows,
		&cols,
		0
	);
	primary_x = columns-cols;

	apply_vertical_edgewins(
		&g_map.right_edgewins, 
		key_regs,
		primary_y,
		columns, 
		rows,
		&cols,
		1
	);
		
	init_region(
		rows, 
		cols, 
		primary_y, 
		primary_x, 
		rows,
		cols,
		primary
	);

	return 0;
}


/* rows: 	the number of rows available
 * cols: 	the number of columns available
 * y:		the starting y coord
 * x:		the starting x coord
 * row_size:the row size requested for the region
 * col_size:the column size requested for the region
 * region:	output parameter
 */
static int init_region(int rows, int cols, int y, int x, int row_size, int col_size, struct aug_region *region) {
	if(rows < 0 || cols < 0 || row_size < 1 || col_size < 1) {
		region->rows = 0;
		region->cols = 0;

		return -1;
	}
	else {
		region->rows = row_size; 
		region->cols = col_size;
		region->y = y;
		region->x = x;

		return 0;
	}
}
