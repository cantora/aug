#ifndef AUG_TOK_ITR
#define AUG_TOK_ITR

#include <string.h>

struct aug_tok_itr {
	char delim;
	const char *curr;
};

#define TOK_ITR_FOREACH(_str, _str_size, _list, _delim, _tok_itr_ptr) \
	for(tok_itr_init(_tok_itr_ptr, _list, _delim); \
		!tok_itr_end(_tok_itr_ptr) && tok_itr_val_TRUE(_tok_itr_ptr, _str, _str_size); \
		tok_itr_next(_tok_itr_ptr) \
		)

/* initialize iterator on *list*, a null terminated string
 * of tokens seperated by *delim* characters. 
 */
void tok_itr_init(struct aug_tok_itr *itr, const char *list, char delim);

/* returns 0 if not at the end of the list, otherwise it returns
 * non-zero 
 */
int tok_itr_end(const struct aug_tok_itr *itr);

/* increments the iterator to the next token */
void tok_itr_next(struct aug_tok_itr *itr);

/* copies at most *size* characters (including null terminating
 * character) to the memory at *val*. returns 0 if the token was
 * *size*-1 characters or less and returns non-zero if the token
 * did not fit into the memory at *val*  
 */
int tok_itr_val(const struct aug_tok_itr *itr, char *val, size_t size);

static inline int tok_itr_val_TRUE(const struct aug_tok_itr *itr, char *val, size_t size) {
	tok_itr_val(itr, val, size);
	return 1;
}

#endif /* AUG_TOK_ITR */