#pragma once

#include <assert.h>

#define TRACE trace

void trace(const char *, ...);

#define ASSERT assert

#ifdef _MSC_VER
#include <crtdbg.h>
#define DbgCheckHeap() ASSERT(_CrtCheckMemory())
#else
#define DbgCheckHeap()
#endif