#pragma once

#include "token.h"

typedef struct parser_t parser_t;

parser_t *parser_new();

void parser_delete(parser_t *);

int parser_advance(parser_t *, const token_t *);