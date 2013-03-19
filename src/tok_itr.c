/* 
 * Copyright 2012 anthony cantor
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
#include "tok_itr.h"

#include <assert.h>

void tok_itr_init(struct aug_tok_itr *itr, const char *list, char delim) {
	assert(list != NULL);

	itr->delim = delim;
	itr->curr = list;
}

int tok_itr_end(const struct aug_tok_itr *itr) {
	return (itr->curr == NULL);
}

void tok_itr_next(struct aug_tok_itr *itr) {
	assert(itr->curr != NULL);

	itr->curr = strchr(itr->curr, itr->delim);
	if(itr->curr != NULL) 
		itr->curr++;
}

size_t tok_itr_val(const struct aug_tok_itr *itr, char *val, size_t size) {
	char *next;
	size_t amt, len;

	assert(size > 0);
	assert(itr->curr != NULL);

	next = strchr(itr->curr, itr->delim);
	if(next == NULL) 
		len = strlen(itr->curr);
	else if(next == itr->curr)
		len = 0; 
	else {  /* next > itr->curr */ 
		assert(next > itr->curr);
		len = next - itr->curr;
	}
	
	/* only want to copy at most size - 1 bytes because
	 * we need to save the last spot for '\0' */
	size--; 
	amt = (len > size)? size : len;

	if(amt > 0)
		strncpy(val, itr->curr, amt);

	/* just copied bytes 0->(amt-1) for total of *amt* bytes.
	 * now add the null byte to pos at *amt*       */
	val[amt] = '\0';

	return len;
}
