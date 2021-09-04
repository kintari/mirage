#include "scanner.h"
#include "debug.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

struct scanner_t {
	FILE *file;
	char *filename;
	int lookahead[2];
	struct {
		char *ptr;
		size_t len;
	} text_buf;
	size_t text_len;
};

static int read_char(scanner_t *s) {
	int ret = s->lookahead[0];
	if (ret != -1) {
		char ch;
		if (fread(&ch, 1, 1, s->file) != 1)
			ch = -1;
		s->lookahead[0] = s->lookahead[1];
		s->lookahead[1] = ch;
	}
	return ret;
}

scanner_t *scanner_new(FILE *file, const char *filename) {
	scanner_t *scanner = malloc(sizeof(scanner_t));
	scanner->file = file;
	scanner->filename = strdup(filename);
	const size_t text_buf_len = 16;
	scanner->text_buf.ptr = malloc(text_buf_len);
	for (int i = 0; i < scanner->text_buf.len; i++)
		scanner->text_buf.ptr[i] = 0;
	scanner->text_buf.len = text_buf_len;
	read_char(scanner);
	read_char(scanner);
	return scanner;
}

void scanner_delete(scanner_t *scanner) {
	free(scanner->filename);
	free(scanner);
}

#define take_while(s,var,cond) \
	do { \
		int var; \
		while (cond) { \
			if ((s)->text_len == (s)->text_buf.len) { \
				void *ptr = realloc(s->text_buf.ptr, 2 * (s)->text_buf.len); \
				ASSERT(ptr); \
				if (ptr) (s)->text_buf.ptr = ptr; \
			} \
		} \
	} while (0)

int scanner_next(scanner_t *s, token_t *t) {
	if (s->lookahead[0] == -1) {
		return -1;
	}
	else {
		while (s->lookahead[0] != -1) {
			if (isspace(s->lookahead[0])) {																																																																																																																																																																																																																																																																																																																																										
				take_while(s, x, isspace(x));
			}
		}
		return 0;
	}
}