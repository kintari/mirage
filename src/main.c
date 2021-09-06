
#include "scanner.h"

#include <stdio.h>

int main(int argc, const char *argv[]) {
  if (argc != 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		return -1;
	}
	const char *filename = argv[1];
	FILE *file = NULL;
#ifdef _MSC_VER
	fopen_s(&file, filename, "rb");
#else
	file = fopen(filename, "rb");
#endif
	if (!file) {
		perror(filename);
		return -1;
	}
	scanner_t *scanner = scanner_new(file, filename);
	token_t token = { 0 };
	while (scanner_next(scanner, &token) == 0) {
		printf("token: '%s' (%d)\n", token.text, token.type);
		token_free(&token);
	}
	scanner_delete(scanner);
	return 0;	
}