#pragma once

#include <stddef.h>

enum {
	TT_NIL,

	TT_IDENTIFIER,

	TT_LIT_FLOAT,
	TT_LIT_INTEGER,
	TT_LIT_STRING,

	TT_ASSIGN_EQ,
	TT_LPAREN,
	TT_RPAREN,
	TT_LBRACE,
	TT_RBRACE,
	TT_LBRACKET,
	TT_RBRACKET,
	TT_SEMICOLON,
	TT_COLON,
	TT_COMMA,
	TT_DOT,
	TT_PLUS,
	TT_MINUS,
	TT_ARROW,

	TT_KW_FUNCTION,
	TT_KW_RETURN,
	TT_KW_IF,
	TT_KW_ELSE,
	TT_KW_VAR,
	TT_KW_CONST,
	TT_KW_U32,
	TT_KW_STRING,

	TT_CMP_EQ,
	TT_CMP_NEQ,
	TT_CMP_GT,
	TT_CMP_GTE,
	TT_CMP_LT,
	TT_CMP_LTE,

	TT_LOG_OR,
	TT_LOG_AND,
	TT_LOG_NOT,

	TT_BIT_OR,
	TT_BIT_AND,
};

typedef struct token_t {
	char *text;
	size_t len;
	int type;
	int line, col;
} token_t;

void token_free(token_t *);