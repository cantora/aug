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
#include "term.h"

void term_init(struct aug_term_t *term, int rows, int cols) {
	VTermScreen *vts;

	term->master = 0;
	term->vt = vterm_new(rows, cols);
	vterm_parser_set_utf8(term->vt, 1);

	vts = vterm_obtain_screen(term->vt);
	vterm_screen_enable_altscreen(vts, 1);
	vterm_screen_reset(vts);	
}

void term_set_callbacks(struct aug_term_t *term, const VTermScreenCallbacks *callbacks, 
							const aug_term_io_callbacks_t *io_callbacks, void *user) {
	VTermScreen *vts;

	vts = vterm_obtain_screen(term->vt);
	vterm_screen_set_callbacks(vts, &callbacks, user);
}

void term_set_master(struct aug_term_t *term, int master) {
	term->master = master;
}

void 
