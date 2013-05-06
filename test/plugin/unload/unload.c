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
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <ccan/tap/tap.h>
#include <ccan/array_size/array_size.h>

const char aug_plugin_name[] = "unload";

static const struct aug_api *g_api;
static struct aug_plugin *g_plugin;

static pthread_t g_tid1;

static void *thread1(void *user) {
	(void)(user);

	diag("++++thread1 (unload)++++");
	pass("thread1(unload) running");
	(*g_api->unload)(g_plugin);
	diag("----thread1 (unload)----\n#");
	return NULL;
}

int aug_plugin_init(struct aug_plugin *plugin, const struct aug_api *api) {
	diag("++++plugin_init (unload)++++");
	g_plugin = plugin;	
	g_api = api;

	(*g_api->log)(g_plugin, "init\n");

	diag("create thread for unloading");
	if(pthread_create(&g_tid1, NULL, thread1, NULL) != 0) {
		diag("expected to be able to create unload thread. abort...");
		return -1;
	}
	pass("created unload thread");

	diag("----plugin_init (unload)----\n#");

	return 0;
}

void aug_plugin_free() {
	int status;

	diag("++++plugin_free (unload)++++");
	(*g_api->log)(g_plugin, "free\n");

	ok1(pthread_equal(pthread_self(), g_tid1) == 0);
	status = pthread_join(g_tid1, NULL);
	if(status != 0)
		fail("pthread join of unload thread failed: %s", strerror(status));
	else 
		pass("pthread_join(...) == 0 (unload)");

	diag("----plugin_free (unload)----\n#");
}