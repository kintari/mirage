
#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#ifdef _MSC_VER
void OutputDebugStringA(const char *);
#endif

char *vasprintf(const char *format, va_list args) {
	char *buf = NULL;
	va_list args_copy;
	va_copy(args_copy, args);
	size_t count = vsnprintf(NULL, 0, format, args);
	if (count > 0) {
		size_t buf_len = count + 1;
		if ((buf = malloc(buf_len)) != NULL) {
			memset(buf, 0, buf_len);
			vsnprintf(buf, buf_len, format, args);
		}
	}
	return buf;
}

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