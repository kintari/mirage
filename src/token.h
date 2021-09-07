#pragma once

#include <stddef.h>

#define TOKEN_TYPES \
	X(TT_NIL) \
	X(TT_IDENTIFIER) \
	X(TT_LIT_FLOAT) \
	X(TT_LIT_INTEGER) \
	X(TT_LIT_STRING) \
	X(TT_ASSIGN_EQ) \
	X(TT_LPAREN) \
	X(TT_RPAREN) \
	X(TT_LBRACE) \
	X(TT_RBRACE) \
	X(TT_LBRACKET) \
	X(TT_RBRACKET) \
	X(TT_SEMICOLON) \
	X(TT_COLON) \
	X(TT_COMMA) \
	X(TT_DOT) \
	X(TT_PLUS) \
	X(TT_MINUS) \
	X(TT_ARROW) \
	X(TT_KW_FUNCTION) \
	X(TT_KW_RETURN) \
	X(TT_KW_IF) \
	X(TT_KW_ELSE) \
	X(TT_KW_VAR) \
	X(TT_KW_CONST) \
	X(TT_KW_U32) \
	X(TT_KW_STRING) \
	X(TT_CMP_EQ) \
	X(TT_CMP_NEQ) \
	X(TT_CMP_GT) \
	X(TT_CMP_GTE) \
	X(TT_CMP_LT) \
	X(TT_CMP_LTE) \
	X(TT_LOG_OR) \
	X(TT_LOG_AND) \
	X(TT_LOG_NOT) \
	X(TT_BIT_OR) \
	X(TT_BIT_AND)

#define X(t) t,

enum {
	TOKEN_TYPES
};

#undef X


typedef struct token_t {
	char *text;
	size_t len;
	int type;
	int line, col;
} token_t;

void token_free(token_t *);

const char *token_type_str(int type);