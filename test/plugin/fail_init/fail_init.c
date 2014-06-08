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
#include "aug_plugin.h"
#include <stdlib.h>
#include <ccan/tap/tap.h>

const char aug_plugin_name[] = "fail_init";

int aug_plugin_start() {
	return -1; /* fail to init */
}

void aug_plugin_free() {
	aug_log("shouldnt have gotten here!\n");
	fail("init failed so execution shouldnt have gotten to this point");
}
