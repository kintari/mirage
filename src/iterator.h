#pragma once

#include "object.h"
#include "debug.h"

#include <stdbool.h>

typedef struct iterator_t iterator_t;

typedef struct iterator_t {
	object_t object;
	bool (*done)(iterator_t *);
	void (*advance)(iterator_t *);
	void *(*value)(iterator_t *);
	object_t *iterable;
	void *context;
} iterator_t;

iterator_t *iterate_impl(object_t *obj);

#define iterate(x) iterate_impl((object_t *)(x))

void *value(iterator_t *iter);

bool done(iterator_t *iter);

void advance(iterator_t *iter);
