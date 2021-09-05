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

static void take(scanner_t *s) {
	if (s->text_len + 1 >= s->text_buf.len) {
		size_t len = 2 * s->text_buf.len;
		void *ptr = realloc(s->text_buf.ptr, len);
		ASSERT(ptr);
		if (ptr) {
			s->text_buf.ptr = ptr;
			s->text_buf.len = len;
		}
		else {
			fputs("out of memory\n", stderr);
			fflush(stderr);
			abort();
		}
	}
	s->text_buf.ptr[s->text_len++] = (char) read_char(s);
}

#define drop_while(s,var,cond) \
	do { \
		int var; \
		while ((var = (s)->lookahead[0]) != -1 && (cond)) { \
			read_char(s); \
		} \
	} while (0)

#define take_while(s,var,cond) \
	do { \
		int var; \
		while ((var = (s)->lookahead[0]) != -1 && (cond)) { \
			take(s); \
		} \
	} while (0)

int scan_inner(scanner_t *s) {
	while (s->lookahead[0] != -1) {
		if (isspace(s->lookahead[0])) {
			drop_while(s, x, isspace(x));
		}
		else if (s->lookahead[0] == '/' && s->lookahead[1] == '/') {
			drop_while(s, x, x != '\n');
		}
		else if (s->lookahead[0] == '/' && s->lookahead[1] == '*') {
			drop_while(s, _, s->lookahead[0] != '*' || s->lookahead[1] != '/');
			read_char(s);
			read_char(s);
		}
		else if (isalpha(s->lookahead[0]) || s->lookahead[0] == '_') {
			take_while(s, x, isalpha(s->lookahead[0]) || isdigit(s->lookahead[0]) || s->lookahead[0] == '_');
			return 1;
		}
		else {
			take(s);
			return 1;
		}
	}
	return s->text_len > 0;
}

int scanner_next(scanner_t *s, token_t *t) {
	if (s->lookahead[0] == -1) {
		return 0;
	}
	else {
		if (t->text == 0) {
			size_t len = 16;
			s->text_buf.ptr = malloc(len);
			s->text_buf.len = len;
		}
		s->text_len = 0;
		int res = scan_inner(s);
		if (res) {
			t->text = s->text_buf.ptr;
			t->len = s->text_len;
			t->type = 0;
			s->text_buf.ptr = 0;
			s->text_buf.len = 0;
			s->text_len = 0;
		}
		return res;
	}
}