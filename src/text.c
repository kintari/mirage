#include "text.h"
#include "debug.h"

#include <malloc.h>
#include <string.h>

struct text_t {
	char *buf;
	size_t len; // length of the string, in characters, not including null terminator
};

static const text_t TEXT_EMPTY = { .buf=NULL, .len=0 };

text_t *text_new() {
	text_t *text = malloc(sizeof(text_t));
	text->buf = malloc(1);
	text->buf[0] = 0;
	text->len = 0;
	return text;
}

text_t *text_new_from_cstr(const char *cstr) {
	text_t *text = NULL;
	if (cstr) {
		text = malloc(sizeof(text_t));
		size_t len = strlen(cstr);
		text->buf = malloc(len + 1);
		memcpy(text->buf, cstr, len);
		text->buf[len] = 0;
		text->len = len;
	}
	return text;
}

text_t *text_copy(const text_t *text) {
	text_t *copy = NULL;
	if (text) {
		// make a copy of text->buf
		char *buf = malloc(text->len + 1);
		memcpy(buf, text->buf, text->len);
		buf[text->len] = 0;
		// construct new text object
		copy = malloc(sizeof(text_t));
		copy->buf = buf;
		copy->len = text->len;
	}
	return copy;
}

void text_delete(text_t *text) {
	if (text) {
		free(text->buf);
		free(text);
	}
}

size_t text_length(const text_t *t) {
	ASSERT(t);
	return t->len;
}

const char *text_buf(const text_t *text) {
	return text ? text->buf : NULL;
}

text_t *text_append(const text_t *text, int ch) {
	ASSERT(text);
	ASSERT(ch <= 256); // FIXME
	char *buf = malloc(text->len + 2);
	ASSERT(buf);
	if (buf) {
		for (int i = 0; i < text->len; i++)
			buf[i] = text->buf[i];
		buf[text->len] = (char) ch;
		buf[text->len + 1] = 0;
		text_t *ret = malloc(sizeof(text_t));
		ASSERT(ret);
		if (ret) {
			ret->buf = buf;
			ret->len = text->len + 1;
			return ret;
		}
	}
	return NULL;
}

text_t *text_concat_cstr(const text_t *text, const char *pstr, size_t pstr_len) {
	size_t len = text->len + pstr_len;
	char *buf = malloc(len + 1);
	ASSERT(buf);
	if (buf) {
		char *pch = buf;
		for (size_t i = 0; i < text->len; i++)
			*(pch++) = text->buf[i];
		for (size_t j = 0; j < pstr_len; j++)
			*(pch++) = pstr[j];
		*pch = 0;
		text_t *ret = malloc(sizeof(text_t));
		if (ret) {
			ret->buf = buf;
			ret->len = len;
			return ret;
		}
		free(buf);
	}
	return NULL;
}

text_t *text_concat(const text_t *t0, const text_t *t1) {
	return text_concat_cstr(t0, t1->buf, t1->len);
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

text_t *text_escape(const text_t *text) {
	text_t *ret = NULL;
	if (text) {
		ret = text_new();
		for (size_t i = 0; i < text->len; i++) {
			char buf[3] = { 0, 0, 0 };
			escape_char(buf, text->buf[i]);
			size_t len = 0;
			if (buf[0]) len++;
			if (buf[1]) len++;
			text_t *tmp = ret;
			ret = text_concat_cstr(ret, buf, len);
			text_delete(tmp);
		}
	}
	return ret;
}
