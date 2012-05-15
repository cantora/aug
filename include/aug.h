#ifndef AUG_AUG_H
#define AUG_AUG_H

#include <wchar.h>
#include <stdint.h>

/* defined below */
struct aug_api_t;
struct aug_plugin_t;

typdef uint32_t aug_attr_t;

typedef enum aug_action_t = { AUG_ACT_OK = 0, AUG_ACT_CANCEL };

/* callbacks which a plugin can register
 * interest in. unless otherwise specified
 * the output parameters of all the callbacks
 * will be initialized to current/default 
 * values, so a simple { return; } function
 * pointer will be the same as a NULL 
 * callback. 
 */
struct aug_plugin_cb_t {
	/* called when a unit of input
	 * is received from stdin. the plugin can
	 * update the ch variable and set action
	 * to AUG_ACT_OK in order to change the
	 * data being sent to the terminal. or 
	 * the plugin can set action to 
	 * AUG_ACT_CANCEL to cause the input
	 * to be filtered from the terminal. */
	void (*input_char)(const struct aug_api_t *api, struct aug_plugin_t *plugin, 
			aug_action_t *action, int *ch);
	
	/* called when a cell on the screen is
	 * about to be updated. the plugin can
	 * alter (or not) any of the output 
	 * variables and then
	 * set action to AUG_ACTION_OK to modify
	 * the cell getting updated. or it can
	 * set action to AUG_ACTION_CANCEL and 
	 * and prevent the cell from being 
	 * updated. */
	void (*update_cell)(const struct aug_api_t *api, struct aug_plugin_t *plugin, 
			aug_action_t *action, int *row, int *col, wchar_t *wch, 
			aug_attr_t *attr, int *color_pair);

	/* called when the cursor is about to be
	 * moved to some location on the screen.
	 * the plugin can change (or not) the 
	 * new_row, new_col output parameters
	 * and set action to AUG_ACT_OK to modify
	 * where the cursor is moved to. */
	void (*move_cursor)(const struct aug_api_t *api, struct aug_plugin_t *plugin,
			aug_action_t *action, int old_row, int old_col, int *new_row, int *new_col);
};

/* this structure represents the 
 * application's view of the plugin
 */
struct aug_plugin_t {
	/* name of the plugin. this name
	 * will be used for option parsing
	 * and configuration file parsing
	 * to automatically provide this 
	 * plugin with user specified
	 * configuration values. 
	 * this should ONLY be modified
	 * in the plugin_init function */
	char *name;
	
	/* callback subscriptions for this
	 * plugin. this structure will be
	 * initialized with null function
	 * ptrs and the plugin_init function
	 * should override the pointers for 
	 * which the plugin wants to receive
	 * callbacks.
	 * this should ONLY be modifed in
	 * the plugin_init function */
	struct aug_plugin_cb_t callbacks;
	
	/* a pointer to keep a private plugin
	 * data structure. the core application
	 * doesnt touch this, so the plugin can 
	 * modify it at any time. 	*/
	void *user;
};

/* passed to the plugin to provide 
 * an interface to the core application.
 */
struct aug_api_t {

	/* call this function to query for a
	 * user specified configuration value
	 * parsed from the command line or the
	 * configuration file. name is the name
	 * of the plugin, key is name of the 
	 * configuration variable, and val is 
	 * a pointer to the location of the
	 * variable value. */
	void (*aug_api_conf)(const char *name, const char *key, const void **val);
	
	/* query for the number of plugins
	 * on the plugin stack */
	void (*aug_api_stack_size)(int *size);

	/* query for the position in the 
	 * plugin stack of a plugin by
	 * name. the plugin at position
	 *  zero gets the first callbacks
	 * and the plugin at the highest
	 * position gets callbacks last. */
	void (*aug_api_stack_pos)(const char *name, int *pos);

};

#endif /* AUG_AUG_H */