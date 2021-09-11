#include "scanner.h"
#include "debug.h"
#include "config.h"
#include "text.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define ALPHA_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT_CHARS "0123456789"
#define SPACE_CHARS " \r\n\t"
#define MISC_CHARS  "(){}[],.;:+-*/"

struct scanner_t {
	FILE *file;
	char *filename;
	int line, col;
	int lookahead[2];
	text_t *text;
};

bool eos(scanner_t *scanner) {
	return scanner->lookahead[0] == -1;
}

static int read_char(scanner_t *s) {
	int ret = s->lookahead[0];
	if (ret != -1) {
		char ch;
		if (fread(&ch, 1, 1, s->file) != 1)
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
		TRACE("read_char() -> '%s'\n", escape_char(buf, ret));
	}
	return ret;
}

scanner_t *scanner_new(FILE *file, const char *filename) {
	scanner_t *scanner = calloc(1,sizeof(scanner_t));
	if (scanner) {
		scanner->file = file;
		scanner->filename = strdup(filename);
		scanner->line = 1;
		scanner->col = 1;
		scanner->text = text_new();
		read_char(scanner);
		read_char(scanner);
	}

	return scanner;
}

void scanner_delete(scanner_t *scanner) {
	text_delete(scanner->text);
	free(scanner->filename);
	free(scanner);
}

static bool peek(scanner_t *scanner, int *pch) {
	int ch = scanner->lookahead[0];
	return ch != -1 ? (*pch = ch), true : false;
}

static bool take(scanner_t *scanner) {
	bool b = !eos(scanner);
	if (b) {
		int ch = read_char(scanner);
		text_t *text = text_append(scanner->text, (char) ch);
		text_delete(scanner->text);
		scanner->text = text;
	}
	return b;
}

void scan_comment(scanner_t *scanner) {
	read_char(scanner);
	read_char(scanner);
	int ch;
	while (peek(scanner, &ch) && ch != '\n')
		read_char(scanner);
}

void scan_multiline_comment(scanner_t *scanner) {
	read_char(scanner);
	read_char(scanner);
	while (!eos(scanner) && !(scanner->lookahead[0] == '*' && scanner->lookahead[1] == '/')) {
		read_char(scanner);
	}
	read_char(scanner);
	read_char(scanner);
}

void scan_digits(scanner_t *scanner) {
	while (!eos(scanner) && isdigit(scanner->lookahead[0]))
		take(scanner);
}

bool scan_numeric_literal(scanner_t *scanner, token_t *token) {
	scan_digits(scanner);
	if (scanner->lookahead[0] == '.' && isdigit(scanner->lookahead[1])) {
		take(scanner);
		scan_digits(scanner);
		token->type = TT_FLOAT_LITERAL;
	}
	else {
		token->type = TT_INTEGER_LITERAL;
	}
	return true;
}

bool scan_text(scanner_t *scanner, token_t *token) {
	int ch;
	while (peek(scanner, &ch) && (isalpha(ch) || isdigit(ch) || ch == '_'))
		take(scanner);
	const char *text = text_buf(scanner->text);
	const struct {
		const char *type_str;
		int type;
	} keywords[] = {
		{ "u32", TT_KW_U32 },
		{ "string", TT_KW_STRING },
		{ "function", TT_KW_FUNCTION },
		{ "if", TT_KW_IF },
		{ "else", TT_KW_ELSE },
		{ "var", TT_KW_VAR },
		{ "const", TT_KW_CONST },
		{ "return", TT_KW_RETURN },
	};
	for (int i = 0; i < _countof(keywords); i++) {
		if (strcmp(text, keywords[i].type_str) == 0) {
			token->type = keywords[i].type;
			return true;
		}
	}
	token->type = TT_IDENTIFIER;
	return true;
}

bool scan_misc(scanner_t *scanner, token_t *token) {
	struct {
		const char text[3];
		int type;
	} misc_tokens[] = {
		{ "==", TT_CMP_EQ },
		{ "||", TT_LOG_OR },
		{ "|=", TT_BIT_OR_EQUALS },
		{ "|", TT_BIT_OR },
		{ "|", TT_BIT_AND },
		{ "(", TT_RPAREN },
		{ ")", TT_LPAREN },
		{ "{", TT_LBRACE },
		{ "}", TT_RBRACE },
		{ ":", TT_COLON },
		{ ";", TT_SEMICOLON },
		{ ",", TT_COMMA },
		{ "-", TT_MINUS },
		{ "+", TT_PLUS },
		{ "=", TT_ASSIGN_EQ },
	};
	for (size_t i = 0; i < _countof(misc_tokens); i++) {
		const char *text = misc_tokens[i].text;
		if (text[1]) {
			// two character token
			if (text[0] == scanner->lookahead[0] && text[1] == scanner->lookahead[1]) {
				token->type = misc_tokens[i].type;
				take(scanner);
				take(scanner);
				return true;
			}
		}
		else {
			// one character token
			if (text[0] == scanner->lookahead[0]) {
				token->type = misc_tokens[i].type;
				take(scanner);
				return true;
			}
		}
	}
	TRACE("unrecognized token %c, %c\n", scanner->lookahead[0], scanner->lookahead[1]);
	abort();
}

static bool scan_inner(scanner_t *scanner, token_t *token) {
	while (!eos(scanner)) {
		if (isspace(scanner->lookahead[0])) {
			while (!eos(scanner) && isspace(scanner->lookahead[0]))
				read_char(scanner);
		}
		else if (scanner->lookahead[0] == '/' && scanner->lookahead[1] == '/') {
			scan_comment(scanner);
		}
		else if (scanner->lookahead[0] == '/' && scanner->lookahead[1] == '*') {
			scan_multiline_comment(scanner);
		}
		else if (isdigit(scanner->lookahead[0])) {
			return scan_numeric_literal(scanner, token);
		}
		else if (isalpha(scanner->lookahead[0]) || scanner->lookahead[0] == '_') {
			return scan_text(scanner, token);
		}
		else {
			return scan_misc(scanner, token);
		}
	}
	return false;
}

bool scanner_next(scanner_t *scanner, token_t *token) {	
	while (!eos(scanner)) {
		int line = scanner->line, col = scanner->col;
		text_delete(scanner->text);
		scanner->text = text_new();
		if (scan_inner(scanner, token)) {
			token->line = line;
			token->col = col;
			token->text = scanner->text;
			scanner->text = 0;
			return true;
		}
	}
	return false;
}
