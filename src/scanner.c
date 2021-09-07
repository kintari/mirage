#include "scanner.h"
#include "debug.h"
#include "config.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_STATES 64

struct scanner_t {
	FILE *file;
	char *filename;
	int lookahead[2];
	struct {
		char *ptr;
		size_t len;
	} text_buf;
	size_t text_len;
	uint8_t state_tbl[MAX_STATES][256];
	size_t num_states;
	int reject_state, accept_state, start_state;
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

static int new_state(scanner_t *scanner) {	
	int state = (int) scanner->num_states++;
	for (int i = 0; i < 256; i++)
		scanner->state_tbl[state][i] = (uint8_t) scanner->accept_state;
	return state;
}

static void transition(scanner_t *scanner, int from_state, const char *values, int to_state) {
	for (const char *pch = values; *pch != 0; pch++)
		scanner->state_tbl[from_state][*pch] = (uint8_t) to_state;
}

scanner_t *scanner_new(FILE *file, const char *filename) {
	scanner_t *scanner = calloc(1,sizeof(scanner_t));
	if (scanner) {
		scanner->file = file;
		scanner->filename = strdup(filename);
		scanner->text_buf.ptr = NULL;
		scanner->text_buf.len = 0;
		scanner->text_len = 0;

		static const char alpha_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		static const char digit_chars[] = "0123456789";
		static const char space_chars[] = " \r\n\t";
		static const char misc_chars[] = "(){}[],.;:+-*/";

		scanner->reject_state = new_state(scanner);
		scanner->accept_state = new_state(scanner);

		scanner->start_state = new_state(scanner);

		int drop_space_state = new_state(scanner);
		transition(scanner, drop_space_state, space_chars, drop_space_state);

		int drop_line_state = new_state(scanner);
		for (int i = 0; i < 256; i++)
			scanner->state_tbl[drop_line_state][i] = (uint8_t) drop_line_state;
		scanner->state_tbl[drop_line_state]['\n'] = (uint8_t) scanner->accept_state;

		int scan_word_state = new_state(scanner);
		transition(scanner, scan_word_state, alpha_chars, scan_word_state);
		transition(scanner, scan_word_state, digit_chars, scan_word_state);
		transition(scanner, scan_word_state, "_", scan_word_state);

		int scan_integer_state = new_state(scanner);
		transition(scanner, scan_integer_state, digit_chars, scan_integer_state);
		int scan_frac_state = new_state(scanner);
		transition(scanner, scan_integer_state, ".", scan_frac_state);
		transition(scanner, scan_frac_state, digit_chars, scan_frac_state);

		const char *opers[] = {
			"==",
			"||"
		};

		for (size_t i = 0; i < _countof(opers); i++) {
			const char *pch = opers[i];
			int cur_state = scanner->start_state;
			while (*pch != 0) {
				int next_state = scanner->state_tbl[cur_state][*pch];
				if (next_state <= scanner->start_state) {
					next_state = new_state(scanner);
					scanner->state_tbl[cur_state][*pch] = (uint8_t) next_state;
				}
				cur_state = next_state;
				pch++;
			}
		}

		int scan_misc_state = new_state(scanner);

		// default transition out of start is to reject
		for (size_t i = 0; i < 256; i++)
			scanner->state_tbl[scanner->start_state][i] = (uint8_t) scanner->reject_state;
		transition(scanner, scanner->start_state, space_chars, drop_space_state);
		transition(scanner, scanner->start_state, alpha_chars, scan_word_state);
		transition(scanner, scanner->start_state, "_", scan_word_state);
		transition(scanner, scanner->start_state, digit_chars, scan_integer_state);
		transition(scanner, scanner->start_state, misc_chars, scan_misc_state);

		read_char(scanner);
		read_char(scanner);
	}
	return scanner;
}

void scanner_delete(scanner_t *scanner) {
	free(scanner->filename);
	free(scanner);
}

static int take(scanner_t *s) {
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
	char ch = (char) read_char(s);
	s->text_buf.ptr[s->text_len++] = ch;
	return ch;
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

static bool peek(scanner_t *scanner, int *pch) {
	int ch = scanner->lookahead[0];
	return ch != -1 ? (*pch = ch), true : false;
}

bool scanner_fsm(scanner_t *s) {
	int state = s->start_state;
	int ch;
	while (peek(s, &ch) && (state = s->state_tbl[state][ch]) > s->start_state) {
		TRACE("fsm: ch='%c', state=%d\n", ch, state);
		take(s);
	}
	if (state == s->reject_state) {
		int i = 0;
		i = i + 1;
	}
	return state == s->accept_state;
}

bool scanner_next(scanner_t *s, token_t *t) {
	if (s->text_buf.ptr == NULL) {
		size_t buf_len = 16;
		if ((s->text_buf.ptr = malloc(buf_len)) != NULL) {
			memset(s->text_buf.ptr, 0, buf_len);
			s->text_buf.len = buf_len;
		}
	}
	s->text_len = 0;
	bool result = scanner_fsm(s);

	if (result) {
		t->text = s->text_buf.ptr;
		t->len = s->text_len;
	}
	else {
		free(s->text_buf.ptr);
	}

	s->text_buf.ptr = 0;
	s->text_buf.len = 0;
	s->text_len = 0;

	return result;

#if 0
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
		else (s->state_tbl[s->lookahead[0]][s->lookahead[1]] != 0) {

		}
		else if {
			s->state_tbl[s->lookahead[0]]
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
#endif
}
