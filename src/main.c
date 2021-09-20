
#include "scanner.h"
#include "debug.h"
#include "text.h"
#include "parser.h"

#include <stdio.h>
#include <malloc.h>

FILE *open_file(const char *filename, const char *mode) {
	FILE *file = NULL;
#ifdef _MSC_VER
	fopen_s(&file, filename, mode);
#else
	file = fopen(filename, mode);
#endif
	return file;
}

reader_t *reader;
scanner_t *scanner;
parser_t *parser;

typedef struct stdio_reader_t {
	reader_t base;
	char *filename;
	FILE *file;
} stdio_reader_t;

void stdio_reader_destroy(void *ptr) {
	stdio_reader_t *self = ptr;
	free(self->filename);
	if (self->file) fclose(self->file);
	free(self);
}

size_t stdio_reader_read_bytes(void *ptr, void *buf, size_t num_req) {
	stdio_reader_t *self = ptr;
	size_t num_read = fread(buf, 1, num_req, self->file);
	return num_read;
}

const char *stdio_reader_get_name(const void *ptr) {
	const stdio_reader_t *self = ptr;
	return self->filename;
}

reader_t *stdio_reader_open(const char *filename) {
	FILE *file = open_file(filename, "rb");
	if (file) {
		stdio_reader_t *stdio_reader = calloc(1,sizeof(stdio_reader_t));
		stdio_reader->base.destroy = stdio_reader_destroy;
		stdio_reader->base.read_bytes = stdio_reader_read_bytes;
		stdio_reader->base.get_name = stdio_reader_get_name;
		stdio_reader->file = file;
		stdio_reader->filename = strdup(filename);
		return &stdio_reader->base;
	}
	return NULL;
}

int parse_loop() {
	int error;
	token_t token = { 0 };
	while ((error = scanner_next(scanner, &token)) == 0) {
		// print the token
		text_t *text = text_escape(token.text);
		TRACE("token: text='%s', type='%s'\n", text_buf(text), text_buf(token.type));
		text_delete(text);

		// handle the token
		parser_advance(parser, &token);
		
		// free token members
		text_delete(token.text);
		text_delete(token.type);
		token = (token_t) { 0 };
	}
	return error;
}

int parse_file(const char *filename) {
	reader = stdio_reader_open(filename);
	scanner = scanner_new(reader);
	parser = parser_new();
	//int error = parse_loop();
	int error = 0;
	parser_delete(parser);
	scanner_delete(scanner);
	return error;
}

int main(int argc, const char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		return -1;
	}
	else {
		return parse_file(argv[1]);
	}
}