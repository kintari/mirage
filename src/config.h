#pragma once

#include <string.h>

#ifdef _MSC_VER
#define strdup _strdup
#endif

#ifndef countof
#define countof(x) (sizeof(x)/sizeof((x)[0]))
#endif
