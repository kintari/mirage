#pragma once

#include <stddef.h>

typedef struct token_t {
	int type;
	char *text;
	size_t len;
	int line, col;
} token_t;

void token_free(token_t *);