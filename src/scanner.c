#include "scanner.h"
#include "debug.h"
#include "config.h"
#include "text.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_STATES 64

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

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
	if (!eos(scanner)) {
		int ch = read_char(scanner);
		text_t *text = text_append(scanner->text, (char) ch);
		text_delete(scanner->text);
		scanner->text = text;
		return true;
	}
	else {
		return false;
	}
}


static bool scan_inner(scanner_t *scanner, token_t *token) {
	while (!eos(scanner)) {
		if (isspace(scanner->lookahead[0])) {
			int ch;
			while (peek(scanner, &ch) && isspace(ch))
				read_char(scanner);
		}
		else if (scanner->lookahead[0] == '/' && scanner->lookahead[1] == '/') {
			read_char(scanner);
			read_char(scanner);
			int ch;
			while (peek(scanner, &ch) && ch != '\n')
				read_char(scanner);
		}
		else if (scanner->lookahead[0] == '/' && scanner->lookahead[1] == '*') {
			read_char(scanner);
			read_char(scanner);
			while (!eos(scanner) && !(scanner->lookahead[0] == '*' && scanner->lookahead[1] == '/')) {
				read_char(scanner);
			}
			read_char(scanner);
			read_char(scanner);
		}
		else if (isdigit(scanner->lookahead[0])) {
			int ch;
			while (peek(scanner, &ch) && isdigit(ch))
				take(scanner);
			if (scanner->lookahead[0] == '.' && isdigit(scanner->lookahead[1])) {
				take(scanner);
				while (peek(scanner, &ch) && isdigit(ch))
					take(scanner);
				token->type = TT_FLOAT_LITERAL;
			}
			else {
				token->type = TT_INTEGER_LITERAL;
			}
			return true;
		}
		else if (isalpha(scanner->lookahead[0]) || scanner->lookahead[0] == '_') {
			int ch;
			while (peek(scanner, &ch) && (isalpha(ch) || isdigit(ch) || ch == '_'))
				take(scanner);
			token->type = TT_IDENTIFIER;
			return true;
		}
		else {
			take(scanner);
			token->type = 0;
			return true;
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
