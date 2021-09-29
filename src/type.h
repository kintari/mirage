#pragma once

#include <stddef.h>

struct object_t;

typedef void (*destructor_t)(struct object_t *);

struct type_t {
	const char *name;
	size_t size;
	destructor_t destroy;
	const struct allocator_vtbl_t *allocator;
	const struct collection_vtbl_t *collection;
	const struct comparable_vtbl_t *comparable;
	const struct functional_vtbl_t *functional;
	const struct iterable_vtbl_t *iterable;
	const struct iterator_vtbl_t *iterator;
};
