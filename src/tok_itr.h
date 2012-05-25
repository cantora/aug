#ifndef AUG_TOK_ITR
#define AUG_TOK_ITR

struct aug_tok_itr {
	char delim;
	const char *curr;
};

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
}

void tok_itr_val(const struct aug_tok_itr *itr, char *val, size_t size) {
	char *next;
	size_t amt, len;

	assert(size >= 0);
	assert(itr->curr != NULL);

	next = strchr(itr->curr, itr->delim);
	if(next == NULL) {
		len = strlen(itr->curr);
	else if(next <= itr->curr)
		len = 0; /* -> will get bumped up to 1 below */
	else /* next > itr->curr */
		len = next - itr->curr - 1;

	/* add one to len b.c. strlen doesnt count 
	 * terminating null byte. */
	len++;  /* >= 1 */
	amt = (len > size)? size : len;
	strncpy(val, itr->curr, amt);
}

#endif /* AUG_TOK_ITR */