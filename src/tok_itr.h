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
#ifndef AUG_TOK_ITR
#define AUG_TOK_ITR

#include <string.h>

/**
 * struct tok_itr - a structure containing the token iterator state
 * @delim: the token delimiter
 * @curr: a pointer the current token (may be empty)
 */
struct aug_tok_itr {
	char delim;
	const char *curr;
};

/**
 * TOK_ITR_FOREACH - iterate through tokens in a string
 * @str: the current token value for each iteration will be saved here.
 * @str_size: the maximum number of bytes writable to @str
 * @list: a string containing tokens seperated by @delim
 * @delim: the token delimiter
 * @tok_itr_ptr: a pointer to a tok_itr struct
 *
 * This wraps the most straightforward usage of this module in a
 * single macro. It's a for loop, so all the normal for loop
 * operations such as break and continue apply.
 *
 * Example:
 *   char val[32];
 *   struct tok_itr itr;
 *  
 *   TOK_ITR_FOREACH(val, 32, "/bin:/usr/bin:/sbin:/usr/local/bin", ':', &itr) {
 *     printf("token = %s", val);
 *   }
 */
#define TOK_ITR_FOREACH(str, str_size, list, delim, tok_itr_ptr) \
	for(tok_itr_init(tok_itr_ptr, list, delim); \
		!tok_itr_end(tok_itr_ptr) && (tok_itr_val(tok_itr_ptr, str, str_size) || 1); \
		tok_itr_next(tok_itr_ptr) \
		)

/**
 * tok_itr_init - initialize an iterator on a string of tokens seperated 
 * by delimiters.
 * @itr: a pointer to a tok_itr struct.
 * @list: a null terminated string of tokens.
 * @delim: the delimiter which seperates tokens in @list
 *
 * Initializes (or resets) a tok_itr struct with a 
 * particular token string and token delimiter.
 *
 * Example:
 * 	 struct tok_itr itr;
 *   tok_itr_init(&itr, "i=1&article=foo&side=bar", '&');
 */
void tok_itr_init(struct aug_tok_itr *itr, const char *list, char delim);

/**
 * tok_itr_end - checks whether iterator has finished
 * @itr: a pointer to a tok_itr struct
 *
 * This function returns 0 if @itr is not at the end of the list, 
 * otherwise it returns non-zero. 
 * Calling this function on an uninitialized tok_itr 
 * struct is undefined.
 *
 * Example: see tok_itr_val() below.
 */
int tok_itr_end(const struct aug_tok_itr *itr);

/**
 * tok_itr_next - increments the iterator to the next token
 * @itr: a tok_itr struct
 *
 * This function modifies @itr such that a call to 
 * tok_itr_val() will return the value of the next
 * token (provided tok_itr_end() returns 0).
 *
 * Calling this function on an uninitialized tok_itr 
 * struct is undefined.
 *
 * Example: see tok_itr_val() below.
 */
void tok_itr_next(struct aug_tok_itr *itr);

/**
 * tok_itr_val - returns the current token value
 * @itr: a pointer to a tok_itr struct
 * @val: a pointer to a character buffer
 * @size: the size of @val in bytes
 *
 * This function copies at most @size characters (including null 
 * terminating character) to the memory at @val. It returns the 
 * length of the token. If the return value is greater than or
 * equal to @size then @val holds the truncated value of the token. 
 *
 * Calling this function on a tok_itr struct for which tok_itr_end()
 * has returned non-zero (and which has not be subsequently 
 * re-initialized) is undefined.
 *
 * Example:
 *   size_t len;
 *   char val[12];
 *   struct tok_itr itr;
 *   char str[] = "/bin:/usr/bin:/usr/local/sbin";
 *
 *   for(tok_itr_init(&itr, str, ':'); tok_itr_end(&itr) == 0; tok_itr_next(&itr) ) {
 *     len = tok_itr_val(&itr, val, 12);
 *     if(len >= 12) 
 *       printf("truncated ");
 *     printf("token = %s\n", val);
 *   }
 */
size_t tok_itr_val(const struct aug_tok_itr *itr, char *val, size_t size);

#endif /* AUG_TOK_ITR */
