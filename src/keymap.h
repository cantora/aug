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
#ifndef AUG_KEYMAP_H
#define AUG_KEYMAP_H

#include "aug.h"
#include "lock.h"
#include <ccan/avl/avl.h>

struct aug_keymap {
	AVL *avl;
	AUG_LOCK_MEMBERS;
};

void keymap_init(struct aug_keymap *map);
void keymap_free(struct aug_keymap *map);

/* bind an aug_on_key_fn to a key extension */
void keymap_bind(struct aug_keymap *map, uint32_t ch, aug_on_key_fn on_key,
					void *user);

/* get the binding for a key extension */
void keymap_binding(struct aug_keymap *map, uint32_t ch, aug_on_key_fn *on_key,
					void **user);

/* unbind a particular key extension. if non-zero if returned,
 * the key extension did not exist.
 */
int keymap_unbind(struct aug_keymap *map, uint32_t ch);

size_t keymap_size(const struct aug_keymap *map);

#endif