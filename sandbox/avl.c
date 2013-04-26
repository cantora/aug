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
#include <stdio.h>
#include <ccan/avl/avl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static inline int addrcmp(const void *a, const void *b) {
	if(a < b)
		return -1;
	else if(a == b)
		return 0;
	else
		return 1;
}

int main(void)
{
	AVL	*avl;
	AvlIter itr;
	size_t i;
	int arr[] = {1001, 4, 2, 58, 233, 128};
	
	avl = avl_new((AvlCompare) addrcmp);

	for(i = 0; i < sizeof(arr)/sizeof(int); i++) {
		avl_insert(avl, 
					(void *) (intptr_t) arr[i], 
					(void *) (main+i) );	
	}

	avl_foreach(itr, avl) {
		printf("%p => %p\n", itr.key, itr.value);
	}

	avl_free(avl);

	return 0;
}