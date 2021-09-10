
#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#ifdef _MSC_VER

extern void OutputDebugStringA(const char *);

char *vasprintf(const char *format, va_list args) {
	char *buf = NULL;
	va_list args_copy;
	va_copy(args_copy, args);
	size_t count = vsnprintf(format, args);
	if (count > 0) {
		size_t buf_len = count + 1;
		if ((buf = malloc(buf_len)) != NULL) {
			memset(buf, 0, buf_len);
			_vsnprintf_s(buf, buf_len, count, format, args);
		}
	}
	return buf;
}

static void output(const char *buf) {
	fputs(buf, stdout);
	OutputDebugStringA(buf);
}

#endif

void trace(const char *format, ...) {
	va_list args;
	va_start(args, format);
#ifdef _MSC_VER
	char *buf = NULL;
	if ((buf = vasprintf(format, args)) != NULL) {
		output(buf);
		free(buf);
	}
#else
	vfprintf(stdout, format, args);
#endif
	va_end(args);
}