#ifndef AUG_PANEL_STACK_H
#define AUG_PANEL_STACK_H

#include "aug.h"

/* iterates through each visible panel */
#define PANEL_STACK_FOREACH(_panel_ptr) \
	for( \
		panel_stack_top( &(_panel_ptr) ); \
		(_panel_ptr) != NULL; \
		(_panel_ptr) = panel_below(_panel_ptr) \
	) 

/* iterates through each visible panel in 
 * a manner such that it is safe to call
 * panel_stack_rm(_panel_ptr) in the loop.
 */
#define PANEL_STACK_FOREACH_SAFE(_panel_ptr, _next_ptr) \
	for( \
		panel_stack_top( &(_panel_ptr) ), (_next_ptr) = panel_below(_panel_ptr); \
		(_panel_ptr) != NULL; \
		(_panel_ptr) = (_next_ptr), (_next_ptr) = panel_below(_next_ptr) \
	) 

/* create a new panel/window pair on top of the
 * stack associated with *plugin*.   */
void panel_stack_push(struct aug_plugin *plugin, 
						int nlines, int ncols, int begin_y, int begin_x);

/* set *panel* to point to the top visible panel. */
void panel_stack_top(PANEL **panel);
void panel_stack_bottom(PANEL **panel);

/* get the plugin associated with the given panel */
void panel_stack_plugin(const PANEL *panel, const struct aug_plugin **plugin);

/* set *size* to the number of visible panels. */
void panel_stack_size(int *size);

/* remove *panel* from the panel stack and delete its
 * associated window */
void panel_stack_rm(PANEL *panel);

/* update the panels */
void panel_stack_update();

#endif /* AUG_PANEL_STACK_H */