#include "token.h"
#include "debug.h"

#include <stdlib.h>

void token_clear(token_t *token) {
	ASSERT(token);
	if (token) {
		text_delete(token->text);
		*token = (token_t) { 0 };
	}
}

#define X(t) { .type=t, .type_str=#t },

const token_typeinfo_t token_typeinfo[TT_COUNT+1] = {
	TOKEN_TYPES
};

#undef X