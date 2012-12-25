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

#ifndef AUG_REGION_MAP_H
#define AUG_REGION_MAP_H

#include <ccan/avl/avl.h>

struct aug_region {
	int y;
	int x;
	int rows;
	int cols;
};

void region_map_init();
void region_map_free();
void region_map_push_top(const void *key, int nlines);
void region_map_push_bot(const void *key, int nlines);
void region_map_push_left(const void *key, int ncols);
int region_map_top_size();
int region_map_bot_size(); 
int region_map_left_size();
int region_map_delete(const void *key);
AVL *region_map_key_regs_alloc();
void region_map_key_regs_free(AVL *key_regs);
void region_map_key_regs_clear(AVL *key_regs);
int region_map_apply(int lines, int columns, AVL *key_regs, struct aug_region *primary);

#endif /* AUG_REGION_MAP_H */