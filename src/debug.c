
#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#ifdef _MSC_VER
void OutputDebugStringA(const char *);
#endif

static void output(const char *buf) {
	fputs(buf, stdout);
#ifdef _MSC_VER
	OutputDebugStringA(buf);
#endif
}

void trace(const char *format, ...) {
	va_list args, args_copy;
	va_start(args, format);
	va_copy(args_copy, args);
	int count = vsnprintf(NULL, 0, format, args_copy);
	va_end(args_copy);
	if (count > 0) {
		int buf_len = count + 1;
		char *buf = calloc(buf_len, 1);
		if (buf) {
			_vsnprintf_s(buf, buf_len, _TRUNCATE, format, args);
			output(buf);
			free(buf);
		}
	}
	va_end(args);
}