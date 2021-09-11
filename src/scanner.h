#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "config.h"
#include "token.h"

typedef struct scanner_t scanner_t;

scanner_t *scanner_new(FILE *file, const char *filename);

void scanner_delete(scanner_t *);

bool scanner_next(scanner_t *, token_t *);