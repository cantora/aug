#ifndef AUG_AUG_TYPES_H
#define AUG_AUG_TYPES_H

#include <stdint.h>
#include "ncurses.h"

#define AUG_MAX_CHARS_PER_CELL 6
struct aug_cell {
	uint32_t chs[AUG_MAX_CHARS_PER_CELL];
	attr_t screen_attrs;
	int color_pair;
};

#endif /* AUG_AUG_TYPES_H */
