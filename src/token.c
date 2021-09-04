#include "token.h"
#include "debug.h"

#include <stdlib.h>

void token_free(token_t *token) {
	ASSERT(token);
	if (token) {
		token->type = 0;
		free(token->text);
		token->text = 0;
		token->len = 0;
		token->line = 1;
		token->col = 0;
	}
}