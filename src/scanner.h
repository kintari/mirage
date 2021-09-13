#pragma once

#include <stdio.h>

#include "config.h"
#include "token.h"

typedef struct reader_t {
	void (*destroy)(void *ctx);
	size_t (*read_bytes)(void *ctx, void *buf, size_t num_req);
	const char *(*get_name)(const void *ctx);
} reader_t;

typedef struct scanner_t scanner_t;

scanner_t *scanner_new(reader_t *);

void scanner_delete(scanner_t *);

int scanner_next(scanner_t *, token_t *);