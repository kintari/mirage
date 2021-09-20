#include "scanner.h"
#include "debug.h"
#include "config.h"
#include "text.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define ALPHA_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT_CHARS "0123456789"
#define SPACE_CHARS " \r\n\t"
#define MISC_CHARS  "(){}[],.;:+-*/"

struct scanner_t {
	reader_t *reader;
	int line, col;
	int lookahead[2];
};

static bool eos(scanner_t *scanner) {
	return scanner->lookahead[0] == -1;
}

static int read_char(scanner_t *s) {
	int ret = s->lookahead[0];
	if (ret != -1) {
		char ch;
		if (s->reader->read_bytes(s->reader, &ch, 1) != 1)
			ch = -1;
		s->lookahead[0] = s->lookahead[1];
		s->lookahead[1] = ch;
		if (ret == '\n') {
			s->line++;
			s->col = 1;
		}
		else {
			s->col++;
		}
	}
	if (0) {
		char buf[3];
		escape_char(buf, ret);
		TRACE("read_char() -> '%s', lookahead=[", escape_char(buf, ret));
		if (s->lookahead[0] == -1)
			TRACE("-1");
		else
			TRACE("'%s'", escape_char(buf, s->lookahead[0]));
		TRACE(", ");
		if (s->lookahead[1] == -1)
			TRACE("-1");
		else
			TRACE("'%s'", escape_char(buf, s->lookahead[1]));
		TRACE("]\n");
	}
	return ret;
}

#define take_while(s,t,p) \
	do { \
		while (!eos(s) && (p)) { \
			take(s,t); \
		} \
	} while (0)

#define drop_while(s,p) \
	do { \
		while (!eos(s) && (p)) { \
			drop(s); \
		} \
	} while (0)

scanner_t *scanner_new(reader_t *reader) {
	scanner_t *scanner = calloc(1,sizeof(scanner_t));
	if (scanner) {
		scanner->reader = reader;
		scanner->line = 1;
		scanner->col = 1;
		read_char(scanner);
		read_char(scanner);
	}
	return scanner;
}

void scanner_delete(scanner_t *scanner) {
	if (scanner->reader->destroy) scanner->reader->destroy(scanner->reader);
}

static bool take(scanner_t *scanner, token_t *token) {
	bool b = !eos(scanner);
	if (b) {
		int ch = read_char(scanner);
		text_t *text = text_append(token->text, (char) ch);
		text_delete(token->text);
		token->text = text;
	}
	return b;
}

static bool drop(scanner_t *scanner) {
	bool b = !eos(scanner);
	if (b) {
		read_char(scanner);
	}
	return b;
}

static void scan_comment(scanner_t *scanner) {
	drop(scanner);
	drop(scanner);
	drop_while(scanner, scanner->lookahead[0] != '\n');
	drop(scanner);
}

static void scan_multiline_comment(scanner_t *scanner) {
	drop(scanner);
	drop(scanner);
	drop_while(scanner, scanner->lookahead[0] != '*' || scanner->lookahead[1] != '/');
	drop(scanner);
	drop(scanner);
}

static void scan_digits(scanner_t *scanner, token_t *token) {
	take_while(scanner, token, isdigit(scanner->lookahead[0]));
}

static void scan_numeric_literal(scanner_t *scanner, token_t *token) {
	scan_digits(scanner, token);
	if (scanner->lookahead[0] == '.' && isdigit(scanner->lookahead[1])) {
		take(scanner, token);
		scan_digits(scanner, token);
		token->type = text_new_from_cstr("FLOAT-LITERAL");;
	}
	else {
		token->type = text_new_from_cstr("INTEGER-LITERAL");
	}
}

static bool is_keyword(const char *text) {
	const char *keywords[] = {
		"u32",
		"string",
		"function",
		"if",
		"else",
		"var",
		"const",
		"return",
	};
	for (int i = 0; i < countof(keywords); i++)
		if (strcmp(text, keywords[i]) == 0)
			return true;
	return false;
}

static void scan_text(scanner_t *scanner, token_t *token) {
	int *pch = &scanner->lookahead[0];
	take_while(scanner, token, isalpha(*pch) || isdigit(*pch) || *pch == '_');
	token->type = is_keyword(text_buf(token->text)) ? text_copy(token->text) : text_new_from_cstr("IDENTIFIER");
}

static int scan_misc(scanner_t *scanner, token_t *token) {
	static const struct {
		const char *text, *type;
	} misc_tokens[] = {
		{ "==", "COMPARE-EQ" },
		{ "||", "LOGICAL-OR" },
		{ "->", "ARROW" },
		{ "|", "BITWISE-OR" },
		{ "|", "BITWISE-AND" },
		{ "(", "LPAREN" },
		{ ")", "RPAREN" },
		{ "{", "LBRACE" },
		{ "}", "RBRACE" },
		{ ":", "COLON" },
		{ ";", "SEMICOLON" },
		{ ",", "COMMA" },
		{ "-", "MINUS" },
		{ "+", "PLUS" },
		{ "=", "ASSIGN-EQUALS" },
		{ ".", "DOT" },
	};
	for (size_t i = 0; i < countof(misc_tokens); i++) {
		const char *text = misc_tokens[i].text;
		if (text[0]) {
			if (text[1] == 0) {
				if (text[0] == scanner->lookahead[0]) {
					take(scanner, token);
					token->type = text_new_from_cstr(misc_tokens[i].text);
					return 0;
				}
			}
			else if (text[2] == 0) {
				if (text[0] == scanner->lookahead[0] && text[1] == scanner->lookahead[1]) {
					take(scanner, token);
					take(scanner, token);
					token->type = text_new_from_cstr(misc_tokens[i].text);
					return 0;
				}
			}
		}
	}
	return -1;
}

static int scan_inner(scanner_t *scanner, token_t *token) {
	while (!eos(scanner)) {
		if (isspace(scanner->lookahead[0])) {
			drop_while(scanner, isspace(scanner->lookahead[0]));
			continue;
		}
		else if (scanner->lookahead[0] == '/' && scanner->lookahead[1] == '/') {
			scan_comment(scanner);
			continue;
		}
		else if (scanner->lookahead[0] == '/' && scanner->lookahead[1] == '*') {
			scan_multiline_comment(scanner);
			continue;
		}
		else if (isdigit(scanner->lookahead[0])) {
			scan_numeric_literal(scanner, token);
			return 0;
		}
		else if (isalpha(scanner->lookahead[0]) || scanner->lookahead[0] == '_') {
			scan_text(scanner, token);
			return 0;
		}
		else {
			return scan_misc(scanner, token);
		}
	}
	return -1;
}

int scanner_next(scanner_t *scanner, token_t *token) {	
	while (!eos(scanner)) {
		int line = scanner->line, col = scanner->col;
		token_t tmp = { .text = text_new(), .type = NULL, .line = line, .col = col };
		int error = scan_inner(scanner, &tmp);
		if (!error) {
			*token = tmp;
		}
		else {
			TRACE("unrecognized token at %s(%d,%d): '%c'\n", scanner->reader->get_name(scanner->reader), line, col, scanner->lookahead[0]);
			text_delete(tmp.text);
		}
		return error;
	}
	return -1;
}
