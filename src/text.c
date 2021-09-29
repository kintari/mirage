#include "text.h"
#include "debug.h"
#include "object.h"
#include "comparable.h"

#include <malloc.h>
#include <string.h>

struct text_t {
	object_t object;
	char *buf;
	size_t len; // length of the string, in characters, not including null terminator
};

static int text_compare(const object_t *x, const object_t *y) {
	return strcmp(cast(x, const text_t *)->buf, cast(y, const text_t *)->buf);
}

const comparable_vtbl_t text_comparable_vtbl = {
	.compare = text_compare
};

const type_t text_type = {
	.comparable = &text_comparable_vtbl
};

static text_t *text_alloc(size_t len) {
	text_t *text = malloc(sizeof(text_t));
	text->object.num_refs = 1;
	text->object.type = &text_type;
	size_t buflen = len + 1;
	text->buf = malloc(buflen);
	memset(text->buf, 0, buflen);
	text->len = len;
	return text;
}

text_t *text_new() {
	return text_alloc(0);
}

text_t *text_new_from_cstr(const char *cstr) {
	text_t *text = NULL;
	if (cstr) {
		size_t len = strlen(cstr);
		text = text_alloc(len);
		memcpy(text->buf, cstr, len);
	}
	return text;
}

text_t *text_copy(const text_t *text) {
	text_t *copy = NULL;
	if (text) {
		// construct new text object
		copy = text_alloc(text->len);
		memcpy(copy->buf, text->buf, text->len);
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
	text_t *ret = text_alloc(text->len + 1);
	memcpy(ret->buf, text->buf, text->len);
	ret->buf[text->len] = (char) ch;
	return ret;
}

text_t *text_concat_cstr(const text_t *text, const char *pstr, size_t pstr_len) {
	text_t *ret = text_alloc(text->len + pstr_len);
	char *pch = ret->buf;
	for (size_t i = 0; i < text->len; i++)
		*(pch++) = text->buf[i];
	for (size_t j = 0; j < pstr_len; j++)
		*(pch++) = pstr[j];
	return ret;
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
