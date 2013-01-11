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

#ifndef AUG_SCREEN_H
#define AUG_SCREEN_H

#include "vterm.h"
#include "term.h"

enum screen_err {
	SCN_ERR_NONE = 0,
	SCN_ERR_INIT,
	SCN_ERR_COLOR_PAIRS,
	SCN_ERR_COLOR_COLORS,
	SCN_ERR_COLOR_NOT_SUPPORTED,
	SCN_ERR_COLOR_NO_DEFAULT,
	SCN_ERR_COLOR_PAIR_INIT,
	SCN_ERR_UNKNOWN
};

int screen_init();
int screen_color_on();
int screen_cleanup();
void screen_free();
void screen_set_term(struct aug_term *term);
void screen_dims(int *rows, int *cols);
int screen_getch(int *ch);
/*void screen_err_msg(int error, char **msg);*/
int screen_color_start();
int screen_damage(VTermRect rect, void *user);
void screen_damage_win();
void screen_redraw();
void screen_refresh();
int screen_movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user);
int screen_bell(void *user);
int screen_settermprop(VTermProp prop, VTermValue *val, void *user);

/* win will be set to an ncurses (WINDOW *), but the idea
 * of this screen module is to encapsulate all the main ncurses
 * action so that a file that includes this header doesnt need
 * to include ncurses 
 */
int screen_push_top_edgewin(int nlines, void **win);

void screen_resize();
int screen_unctrl(int ch, char *str);
void screen_doupdate();
void screen_clear();
int screen_redraw_term_win();

#endif /* AUG_SCREEN_H */