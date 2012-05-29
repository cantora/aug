#ifndef AUG_KEYMAP_H
#define AUG_KEYMAP_H

#include "aug.h"
#include <ccan/avl/avl.h>

struct aug_keymap {
	AVL *avl;
};

void keymap_init(struct aug_keymap *map);
void keymap_free(struct aug_keymap *map);

/* bind an aug_on_key_fn to a key extension */
void keymap_bind(struct aug_keymap *map, int ch, aug_on_key_fn on_key,
					void *user);

/* get the binding for a key extension */
void keymap_binding(struct aug_keymap *map, int ch, aug_on_key_fn *on_key,
					void **user);

/* unbind a particular key extension. if non-zero if returned,
 * the key extension did not exist.
 */
int keymap_unbind(struct aug_keymap *map, int ch);

size_t keymap_size(const struct aug_keymap *map);

#endif