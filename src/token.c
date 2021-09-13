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
