#include "token.h"
#include "debug.h"

#include <stdlib.h>

void token_free(token_t *token) {
	ASSERT(token);
	if (token) {
		text_delete(token->text);
	}
}

#define X(T) case T: return #T;

const char *token_type_str(int type) {
	switch (type) {
		TOKEN_TYPES
	default:
		return NULL;
	}
}

#undef X