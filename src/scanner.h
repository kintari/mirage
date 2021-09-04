#pragma once

#include <stdio.h>

#include "token.h"

typedef struct scanner_t scanner_t;

scanner_t *scanner_new(FILE *file, const char *filename);

void scanner_delete(scanner_t *);

int scanner_next(scanner_t *, token_t *);