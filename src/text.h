#pragma once

typedef struct text_t text_t;

text_t *text_new();

void text_delete(text_t *text);

text_t *text_append(text_t *text, int ch);

text_t *text_clear(text_t *text);

char *text_move(text_t *text);

char *escape_char(char buf [3], int ch);

text_t *text_escape(const char *, size_t num_chars);