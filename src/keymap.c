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
}

void keymap_free(struct aug_keymap *map) {
	AvlIter itr;
	
	avl_foreach(itr, map->avl) {
		free(itr.value);
	}

	avl_free(map->avl);
}

void keymap_bind(struct aug_keymap *map, int ch, aug_on_key_fn on_key,
					void *user) {
	struct aug_keymap_desc *desc;
	
	desc = malloc( sizeof(struct aug_keymap_desc) );
	if(desc == NULL)
		err_exit(0, "memory error!");

	desc->on_key = on_key;
	desc->user = user;

	avl_insert(map->avl, (void *) (intptr_t) ch, desc);
} 

void keymap_binding(struct aug_keymap *map, int ch, aug_on_key_fn *on_key,
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

int keymap_unbind(struct aug_keymap *map, int ch) {
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

