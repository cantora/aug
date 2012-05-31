#ifndef AUG_PANEL_STACK_H
#define AUG_PANEL_STACK_H

#if defined(__APPLE__)
#	include <ncurses.h>
#else
#	include <ncursesw/curses.h>
#endif

#include "aug.h"

#define PANEL_STACK_FOREACH(_panel_ptr) \
	for( \
		panel_stack_top( &(_panel_ptr) ); \
		(_panel_ptr) != NULL; \
		(_panel_ptr) = panel_below(_panel_ptr) \
	) 

#define PANEL_STACK_FOREACH_SAFE(_panel_ptr, _next_ptr) \
	for( \
		panel_stack_top( &(_panel_ptr) ), (_next_ptr) = panel_below(_panel_ptr); \
		(_panel_ptr) != NULL; \
		(_panel_ptr) = (_next_ptr), (_next_ptr) = panel_below(_next_ptr) \
	) 

void panel_stack_push(struct aug_plugin *plugin, 
						int nlines, int ncols, int begin_y, int begin_x);
void panel_stack_top(PANEL **panel);
void panel_stack_plugin(PANEL *panel);
void panel_stack_size(int *size);
void panel_stack_rm(PANEL *panel);
void panel_stack_free();

#endif /* AUG_PANEL_STACK_H */