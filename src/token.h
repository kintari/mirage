#pragma once

#include "text.h"

#include <stddef.h>

typedef struct token_t {
	text_t *type, *text;
	int line, col;
} token_t;

void token_clear(token_t *);