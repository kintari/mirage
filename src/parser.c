#include "parser.h"

#include <stdlib.h>



struct parser_t {
	int state;
};

parser_t *parser_new() {
	parser_t *parser = calloc(1,sizeof(parser_t));
	return parser;
}

void parser_delete(parser_t *parser) {
	free(parser);
}