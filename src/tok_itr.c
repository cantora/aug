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

int tok_itr_val(const struct aug_tok_itr *itr, char *val, size_t size) {
	char *next;
	size_t amt, len;
	int result;

	assert(size > 0);
	assert(itr->curr != NULL);

	next = strchr(itr->curr, itr->delim);
	if(next == NULL) 
		len = strlen(itr->curr);
	else if(next == itr->curr)
		len = 0; /* -> will get bumped up to 1 below */
	else {  /* next > itr->curr */ 
		assert(next > itr->curr);
		len = next - itr->curr;
	}
	
	/* only want to copy at most size - 1 bytes because
	 * we need to save the last spot for '\0' */
	size--; 
	if(len > size) {
		amt = size;
		result = -1;
	}
	else {
		amt = len;
		result = 0;
	}

	if(amt > 0)
		strncpy(val, itr->curr, amt);

	/* just copied bytes 0->(amt-1) for total of *amt* bytes.
	 * now add the null byte to pos at *amt*       */
	val[amt] = '\0';

	return result;
}
