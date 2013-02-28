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
#include "keymap.h"
#include <stdint.h>
#include <stdlib.h>

#include "err.h"

struct aug_keymap_desc {
	aug_on_key_fn on_key;
	void *user;

};

static inline int cmp_addr(const void *a, const void *b) {
	if(a < b)
		return -1;
	else if(a == b)
		return 0;
	else
		return 1;
}

void keymap_init(struct aug_keymap *map) {
	map->avl = avl_new((AvlCompare) cmp_addr);
	AUG_LOCK_INIT(map);
}

void keymap_free(struct aug_keymap *map) {
	AvlIter itr;
	
	avl_foreach(itr, map->avl) {
		free(itr.value);
	}

	avl_free(map->avl);

	AUG_LOCK_FREE(map);
}

void keymap_bind(struct aug_keymap *map, uint32_t ch, aug_on_key_fn on_key,
					void *user) {
	struct aug_keymap_desc *desc;
	
	desc = malloc( sizeof(struct aug_keymap_desc) );
	if(desc == NULL)
		err_exit(0, "memory error!");

	desc->on_key = on_key;
	desc->user = user;

	avl_insert(map->avl, (void *) (intptr_t) ch, desc);
} 

void keymap_binding(struct aug_keymap *map, uint32_t ch, aug_on_key_fn *on_key,
					void **user) {
	struct aug_keymap_desc *desc;

	desc = avl_lookup(map->avl, (void *) (intptr_t) ch);
	if(desc == NULL) {
		*on_key = NULL;
		if(user != NULL)
			*user = NULL;
	}
	else {
		*on_key = desc->on_key;
		if(user != NULL)
			*user = desc->user;
	}
}

int keymap_unbind(struct aug_keymap *map, uint32_t ch) {
	struct aug_keymap_desc *desc;

	desc = avl_lookup(map->avl, (void *) (intptr_t) ch);
	if(desc == NULL) {
		return -1;
	}
	
	if(avl_remove(map->avl, (void *) (intptr_t) ch) == false) 
		err_exit(0, "just found an object, but now cant delete it from avl tree?");

	free(desc);

	return 0;	
}

size_t keymap_size(const struct aug_keymap *map) {
	return avl_count(map->avl);
}

