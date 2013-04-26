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
#ifndef AUG_AUG_PLUGIN_H
#define AUG_AUG_PLUGIN_H

#include "aug.h"

int aug_plugin_api_version_major() {
	return AUG_API_VERSION_MAJOR;
}

int aug_plugin_api_version_minor() {
	return AUG_API_VERSION_MINOR;
}

const char const aug_plugin_name[];
int aug_plugin_init( AUG_API_INIT_ARG_PROTO );
void aug_plugin_free( AUG_API_FREE_ARG_PROTO );

#endif /* AUG_AUG_PLUGIN_H */
