/* 
 * The MIT License (MIT)
 * Copyright (c) 2013 anthony cantor
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef AUG_AUG_PLUGIN_H
#define AUG_AUG_PLUGIN_H

#include "aug.h"
#include <string.h>

int aug_plugin_api_version_major() {
	return AUG_API_VERSION_MAJOR;
}

int aug_plugin_api_version_minor() {
	return AUG_API_VERSION_MINOR;
}

const char const aug_plugin_name[];
int aug_plugin_init( AUG_API_INIT_ARG_PROTO );
void aug_plugin_free( AUG_API_FREE_ARG_PROTO );

static inline void aug_callbacks_init(struct aug_plugin_cb *cbs) {
	memset(cbs, 0, sizeof(*cbs));
}

#endif /* AUG_AUG_PLUGIN_H */
