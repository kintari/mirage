#include "text.h"
#include "debug.h"

#include <malloc.h>
#include <string.h>

struct text_t {
	char *buf;
	size_t len; // length of the string, in characters, not including null terminator
	size_t alloc; // number of bytes allocated in 'buf'
};

static const text_t TEXT_EMPTY = { .buf=NULL, .len=0, .alloc=0 };

text_t *text_new() {
	text_t *text = malloc(sizeof(text_t));
	*text = TEXT_EMPTY;
	return text;
}

void text_delete(text_t *text) {
	if (text) {
		free(text->buf);
		free(text);
	}
}

text_t *text_append(text_t *text, int ch) {
	ASSERT(text);
	ASSERT(ch <= 256); // FIXME
	size_t alloc = text->alloc ? text->alloc + 1 : 2;
	char *buf = realloc(text->buf, alloc);
	ASSERT(buf);
	if (buf) {
		buf[text->len] = (char) ch;
		buf[text->len+1] = 0;
		*text = (text_t) { .buf=buf, .len=text->len+1, .alloc=alloc };
	}
	return text;
}

text_t *text_clear(text_t *text) {
	if (text) {
		free(text->buf);
		*text = TEXT_EMPTY;
	}
	return text;
}

char *text_move(text_t *text) {
	ASSERT(text);
	char *r = text->buf;
	*text = TEXT_EMPTY;
	return r;
}

char *escape_char(char buf[3], int ch) {
	switch (ch) {
		case '\r':
			buf[0] = '\\';
			buf[1] = 'r';
			buf[2] = 0;
			break;
		case '\n':
			buf[0] = '\\';
			buf[1] = 'n';
			buf[2] = 0;
			break;
		case '\t':
			buf[0] = '\\';
			buf[1] = 't';
			buf[2] = 0;
			break;
		case '\0':
			buf[0] = '\\';
			buf[1] = '0';
			buf[2] = 0;
			break;
		default:
			buf[0] = (char) ch;
			buf[1] = 0;
			buf[2] = 0;
			break;
	}
	return buf;
}

text_t *text_escape(const char *str, size_t num_chars) {
	text_t *text = NULL;
	if (str) {
		text = text_new();
		if (num_chars == 0) num_chars = strlen(str);
		for (size_t i = 0; i < num_chars; i++) {
			char buf[3] = { 0, 0, 0 };
			escape_char(buf, str[i]);
			if (buf[0]) {
				text_append(text, buf[0]);
				if (buf[1]) {
					text_append(text, buf[1]);
					if (buf[2]) {
						text_append(text, buf[2]);
					}
				}
			}
		}
	}
	return text;
}
