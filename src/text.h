#pragma once

#include <stddef.h>

typedef struct text_t text_t;

text_t *text_new(void);

text_t *text_new_from_cstr(const char *);

text_t *text_copy(const text_t *);

void text_delete(text_t *text);

size_t text_length(const text_t *);

const char *text_buf(const text_t *);

text_t *text_append(const text_t *text, int ch);

text_t *text_concat(const text_t *, const text_t *);

text_t *text_concat_cstr(const text_t *, const char *, size_t);

text_t *text_escape(const text_t *);

char *escape_char(char buf [3], int ch);
