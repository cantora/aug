/* 
 * Copyright 2013 anthony cantor
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
#ifndef AUG_AUG_API_MACROS_H
#define AUG_AUG_API_MACROS_H

#ifndef AUG_API_HANDLE
#	error "you must define AUG_API_HANDLE to a global aug api handle to use " __FILE__
#endif

#ifndef AUG_PLUGIN_HANDLE
#	error "you must define AUG_PLUGIN_HANDLE to a global handle to your plugin to use " __FILE__
#endif

#define AUG_API_CALL(_fn, ...) \
	(*(AUG_API_HANDLE)->_fn)(__VA_ARGS__)

#define aug_log(...) \
	AUG_API_CALL(log, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_keyname_to_key(...) \
	AUG_API_CALL(keyname_to_key, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_unload() \
	AUG_API_CALL(unload, (AUG_PLUGIN_HANDLE))
#define aug_conf_val(...) \
	AUG_API_CALL(conf_val, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_callbacks(...) \
	AUG_API_CALL(callbacks, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_key_bind(...) \
	AUG_API_CALL(key_bind, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_key_unbind(...) \
	AUG_API_CALL(key_unbind, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_lock_screen() \
	AUG_API_CALL(lock_screen, (AUG_PLUGIN_HANDLE))
#define aug_unlock_screen() \
	AUG_API_CALL(unlock_screen, (AUG_PLUGIN_HANDLE))
#define aug_screen_win_alloc_top(...) \
	AUG_API_CALL(screen_win_alloc_top, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_screen_win_alloc_bot(...) \
	AUG_API_CALL(screen_win_alloc_bot, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_screen_win_alloc_left(...) \
	AUG_API_CALL(screen_win_alloc_left, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_screen_win_alloc_right(...) \
	AUG_API_CALL(screen_win_alloc_right, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_screen_win_dealloc(...) \
	AUG_API_CALL(screen_win_dealloc, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_screen_panel_alloc(...) \
	AUG_API_CALL(screen_panel_alloc, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_screen_panel_dealloc(...) \
	AUG_API_CALL(screen_panel_dealloc, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_screen_panel_size(...) \
	AUG_API_CALL(screen_panel_size, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_screen_panel_update() \
	AUG_API_CALL(screen_panel_update, (AUG_PLUGIN_HANDLE))
#define aug_screen_doupdate() \
	AUG_API_CALL(screen_doupdate, (AUG_PLUGIN_HANDLE) )
#define aug_primary_term_damage(...) \
	AUG_API_CALL(primary_term_damage, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_terminal_new(...) \
	AUG_API_CALL(terminal_new, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_terminal_delete(...) \
	AUG_API_CALL(terminal_delete, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_terminal_pid(...) \
	AUG_API_CALL(terminal_pid, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_terminal_terminated(...) \
	AUG_API_CALL(terminal_terminated, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_terminal_run(...) \
	AUG_API_CALL(terminal_run, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_terminal_input(...) \
	AUG_API_CALL(terminal_input, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_terminal_input_chars(...) \
	AUG_API_CALL(terminal_input_chars, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_primary_input(...) \
	AUG_API_CALL(primary_input, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )
#define aug_primary_input_chars(...) \
	AUG_API_CALL(primary_input_chars, (AUG_PLUGIN_HANDLE), __VA_ARGS__ )

#endif /* AUG_AUG_API_H */