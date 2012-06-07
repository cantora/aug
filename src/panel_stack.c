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
#include "panel_stack.h"

#include "err.h"

void panel_stack_push(struct aug_plugin *plugin, 
						int nlines, int ncols, int begin_y, int begin_x) {
	WINDOW *win;
	PANEL *panel;

	win = newwin(nlines, ncols, begin_y, begin_x);
	if(win == NULL) 
		err_exit(0, "newwin failed!");
		
	panel = new_panel(win);
	if(panel == NULL)
		err_exit(0, "new_panel failed!");

	if(set_panel_userptr(panel, plugin) == ERR)
		err_exit(0, "set_panel_userptr failed!");
}

inline void panel_stack_top(PANEL **panel) {
	*panel = panel_below(NULL);
}

inline void panel_stack_bottom(PANEL **panel) {
	*panel = panel_above(NULL);
}

void panel_stack_plugin(const PANEL *panel, const struct aug_plugin **plugin) {
	*plugin = panel_userptr(panel);
}

void panel_stack_size(int *size) {
	int i;
	PANEL *panel;

	i = 0;
	PANEL_STACK_FOREACH(panel) {
		i++;
	}
	*size = i;
}

void panel_stack_rm(PANEL *panel) {
	WINDOW *win;
	
	if( (win = panel_window(panel) ) == NULL ) 
		err_exit(0, "could not get window from panel");

	if( del_panel(panel) == ERR ) 
		err_exit(0, "could not delete panel");
	
	if( delwin(win) == ERR )
		err_exit(0, "could not delete window");

}

void panel_stack_update() {
	update_panels();
}