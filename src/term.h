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
#ifndef AUG_TERM_H
#define AUG_TERM_H

#include "vterm.h"
#include "lock.h"

struct aug_term_io_callbacks {
	void (*refresh)(void *user);
};

struct aug_term {
	VTerm *vt;
	int master;
	struct aug_term_io_callbacks io_callbacks;
	AUG_LOCK_MEMBERS;
	void *user;
};

void term_init(struct aug_term *term, int rows, int cols);
void term_free(struct aug_term *term);
void term_dims(const struct aug_term *term, int *rows, int *cols);
void term_set_callbacks(struct aug_term *term, const VTermScreenCallbacks *screen_callbacks, 
							const struct aug_term_io_callbacks *io_callbacks, void *user);
void term_clear_callbacks(struct aug_term *term);

/* returns non-zero and sets errno if any errors occur. */
int term_set_master(struct aug_term *term, int master);
int term_resize(struct aug_term *term, int rows, int cols);

#endif /* AUG_TERM_H */