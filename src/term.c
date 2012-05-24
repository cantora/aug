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
#include "attr.h"
#include <sys/ioctl.h>

static int term_resize_master(const struct aug_term_t *);

void term_init(struct aug_term_t *term, int rows, int cols) {
	VTermScreen *vts;
	VTermState *state;

	term->master = 0;
	term->vt = vterm_new(rows, cols);
	state = vterm_obtain_state(term->vt);
	/* have to cast default_color because the api isnt const correct */
	vterm_state_set_default_colors(state, (VTermColor *) &VTERM_DEFAULT_COLOR, (VTermColor *) &VTERM_DEFAULT_COLOR);	
	vterm_parser_set_utf8(term->vt, 1);

	vts = vterm_obtain_screen(term->vt);
	vterm_screen_enable_altscreen(vts, 1);
	vterm_screen_reset(vts);	

	term->user = NULL;
	term->io_callbacks.refresh = NULL;
}

void term_free(struct aug_term_t *term) {
	vterm_free(term->vt);
}

void term_dims(const struct aug_term_t *term, int *rows, int *cols) {
	vterm_get_size(term->vt, rows, cols);
}

void term_set_callbacks(struct aug_term_t *term, const VTermScreenCallbacks *screen_callbacks, 
							const struct aug_term_io_callbacks_t *io_callbacks, void *user) {
	VTermScreen *vts;

	term->user = user;
	vts = vterm_obtain_screen(term->vt);
	vterm_screen_set_callbacks(vts, screen_callbacks, user);
	term->io_callbacks = *io_callbacks;
}

static int term_resize_master(const struct aug_term_t *term) {
	struct winsize size;

	term_dims(term, (int *) &size.ws_row, (int *) &size.ws_col);
	if(ioctl(term->master, TIOCSWINSZ, &size) != 0)
		return -1; 

	return 0;
}

int term_set_master(struct aug_term_t *term, int master) {
	term->master = master;
	if(term_resize_master(term) != 0)
		return -1;

	return 0;
}

int term_resize(struct aug_term_t *term, int rows, int cols) {
	fprintf(stderr, "term: resize %d, %d\n", rows, cols);
	
	/* this should cause full damage and the window will be repainted
	 * by a .damage callback
	 */
	vterm_set_size(term->vt, rows, cols);
	if(term->master != 0) /* only resize the master if we have a valid pty */
		if(term_resize_master(term) != 0)
			return -1;
	
	return 0;
}