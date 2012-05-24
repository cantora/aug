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

struct aug_term_io_callbacks_t {
	void (*refresh)(void *user);
};

struct aug_term_t {
	VTerm *vt;
	int master;
	struct aug_term_io_callbacks_t io_callbacks;
	void *user;
};

void term_init(struct aug_term_t *term, int rows, int cols);
void term_free(struct aug_term_t *term);
void term_dims(const struct aug_term_t *term, int *rows, int *cols);
void term_set_callbacks(struct aug_term_t *term, const VTermScreenCallbacks *screen_callbacks, 
							const struct aug_term_io_callbacks_t *io_callbacks, void *user);
void term_clear_callbacks(struct aug_term_t *term);

/* returns non-zero and sets errno if any errors occur. */
int term_set_master(struct aug_term_t *term, int master);
int term_resize(struct aug_term_t *term, int rows, int cols);

#endif /* AUG_TERM_H */