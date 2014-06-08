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
#ifndef AUG_AUG_API_H
#define AUG_AUG_API_H

#include "aug.h"

#define AUG_API_HANDLE aug_api_handle
#define AUG_PLUGIN_HANDLE aug_plugin_handle
extern const struct aug_api * AUG_API_HANDLE;
extern struct aug_plugin * AUG_PLUGIN_HANDLE;

#include "aug_api_macros.h"

#define AUG_GLOBAL_API_OBJECTS \
	const struct aug_api * AUG_API_HANDLE; \
	struct aug_plugin * AUG_PLUGIN_HANDLE;

#define AUG_API_INIT(plugin, api) \
	do { \
		AUG_API_HANDLE = api; \
		AUG_PLUGIN_HANDLE = plugin; \
	} while(0)

#endif /* AUG_AUG_API_H */
