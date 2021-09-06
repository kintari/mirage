#include "scanner.h"
#include "debug.h"
#include "config.h"

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
	int state_tbl[256][256];
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
	if (scanner) {
		scanner->file = file;
		scanner->filename = strdup(filename);
		scanner->text_buf.ptr = NULL;
		scanner->text_buf.len = 0;
		scanner->text_len = 0;
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
				scanner->state_tbl[i][j] = 0;
		scanner->state_tbl['=']['='] = TT_ASSIGN_EQ;
		read_char(scanner);
		read_char(scanner);
	}
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

static void drop(scanner_t *s) {
	read_char(s);
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

static void scan_word(scanner_t *s, token_t *t) {
	take_while(s, x, isalpha(s->lookahead[0]) || isdigit(s->lookahead[0]) || s->lookahead[0] == '_');
	static const struct {
		const char *text;
		int type;
	} keywords[] = {
		{ "else", TT_KW_ELSE },
		{ "function", TT_KW_FUNCTION },
		{ "if", TT_KW_IF },
		{ "return", TT_KW_RETURN },
		{ "string", TT_KW_STRING },
		{ "u32", TT_KW_U32 },
		{ "var", TT_KW_VAR },
		{ "const", TT_KW_CONST }
	};
	t->type = TT_IDENTIFIER;
	for (size_t i = 0; i < _countof(keywords); i++)
		if (strcmp(s->text_buf.ptr, keywords[i].text) == 0)
			t->type = (int) i;
}

bool scanner_next(scanner_t *s, token_t *t) {
	if (s->text_buf.ptr == NULL) {
		size_t buf_len = 16;
		if ((s->text_buf.ptr = malloc(buf_len)) != NULL) {
			memset(s->text_buf.ptr, 0, buf_len);
			s->text_buf.len = buf_len;
		}
	}
	while (s->lookahead[0] != -1) {
		s->text_len = 0;
		if (isspace(s->lookahead[0])) {
			drop_while(s, x, isspace(x));
			continue;
		}
		else if (s->lookahead[0] == '/' && s->lookahead[1] == '/') {
			drop_while(s, x, x != '\n');
			continue;
		}
		else if (s->lookahead[0] == '/' && s->lookahead[1] == '*') {
			drop_while(s, _, s->lookahead[0] != '*' || s->lookahead[1] != '/');
			drop(s);
			drop(s);
			continue;
		}
		else if (isdigit(s->lookahead[0])) {
			take_while(s, x, isdigit(x));
			if (s->lookahead[0] == '.' && isdigit(s->lookahead[1])) {
				take(s);
				take_while(s, x, isdigit(x));
				t->type = TT_LIT_FLOAT;
			}
			else {
				t->type = TT_LIT_INTEGER;
			}
		}
		else if (isalpha(s->lookahead[0]) || s->lookahead[0] == '_') {
			scan_word(s, t);
		}
		else if (strchr("=!><|&~^+-/*", s->lookahead[0]) && s->lookahead[1] == '=') {
			take(s);
			take(s);
		}
		else if (strchr("&|", s->lookahead[0])  && s->lookahead[1] == s->lookahead[0]) {
			take(s);
			take(s);
		}
		else if (strchr("(){}[],.;:", s->lookahead[0])) {
			take(s);
		}
		else {
			fprintf(stderr, "unexpected character: '%c'\n", s->lookahead[0]);
			return -1;
		}

		t->text = s->text_buf.ptr;
		t->len = s->text_len;

		s->text_buf.ptr = 0;
		s->text_buf.len = 0;
		s->text_len = 0;

		return 0;
	}
	return -1;
}
