
#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#ifdef _MSC_VER

extern void OutputDebugStringA(const char *);

char *vasprintf(const char *format, va_list args) {
	char *buf = NULL;
	size_t count = _vscprintf(format, args);
	if (count > 0) {
		size_t buf_len = count + 1;
		if ((buf = malloc(buf_len)) != NULL) {
			memset(buf, 0, buf_len);
			_vsnprintf_s(buf, buf_len, count, format, args);
		}
	}
	return buf;
}

#endif

static void output(const char *buf) {
	fputs(buf, stdout);
#ifdef _MSC_VER
	OutputDebugStringA(buf);
#endif
}

void trace(const char *format, ...) {
	va_list args;
	va_start(args, format);
	char *buf = NULL;
	if ((buf = vasprintf(format, args)) != NULL) {
		output(buf);
		free(buf);
	}
	va_end(args);
}