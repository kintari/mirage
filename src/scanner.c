#include "scanner.h"

#include <string.h>
#include <stdlib.h>

struct scanner_t {
	FILE *file;
	char *filename;
	int lookahead[2];
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
	read_char(scanner);
	read_char(scanner);
}

void scanner_delete(scanner_t *scanner) {
	free(scanner->filename);
	free(scanner);
}

int scanner_next(scanner_t *s, token_t *t) {
	if (s->lookahead[0] == -1) {
		return -1;
	}
	else {
		while (s->lookahead != -1) {
			if (isspace(s->lookahead[0])) {
				while (isspace(s->lookahead[0])) {
					read_char(s);
				}
			}
		}
	}
}