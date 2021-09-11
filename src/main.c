
#include "scanner.h"
#include "debug.h"
#include "text.h"

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

void scan_loop(scanner_t *scanner, token_t *token) {
	while (scanner_next(scanner, token)) {
		text_t *text = text_escape(token->text);
		TRACE("token: '%s', %s\n", text_buf(text), token_typeinfo[token->type].type_str);
		text_delete(text);
		token_clear(token);
	}
}

int main(int argc, const char *argv[]) {
  if (argc != 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		return -1;
	}
	const char *filename = argv[1];
	reader_t *reader = stdio_reader_open(filename);
	scanner_t *scanner = scanner_new(reader);
	token_t token = { 0 };
	scan_loop(scanner, &token);
	scanner_delete(scanner);
	return 0;
}