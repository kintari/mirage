#pragma once

#include "object.h"
#include "debug.h"

#include <stdbool.h>

typedef struct iterator_t iterator_t;

typedef struct iterator_t {
	object_t object;
	object_t *iterable;
	void *context;
	size_t ordinal;
} iterator_t;

typedef struct iterable_vtbl_t {
	iterator_t *(*iterate)(object_t *);
} iterable_vtbl_t;

typedef struct iterator_vtbl_t {
	bool (*done)(iterator_t *);
	void (*advance)(iterator_t *);
	object_t *(*value)(iterator_t *);
} iterator_vtbl_t;

iterator_t *iterate(object_t *);

object_t *value(iterator_t *iter);

bool done(iterator_t *iter);

void advance(iterator_t *iter);

#define foreach(iterable,identifier) for (iterator_t *identifier = iterate(iterable); !done(identifier); advance(identifier))
