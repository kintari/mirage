#pragma once

#include "debug.h"
#include "object.h"

#include <stdbool.h>

typedef struct collection_vtbl_t {
	bool (*add)(object_t *collection, void *value);
	size_t (*count)(const object_t *collection);
} collection_vtbl_t;

static inline bool add(object_t *obj, void *value) {
	ASSERT(obj);
	const collection_vtbl_t *vtbl = obj->type->collection;
	if (vtbl) {
		return vtbl->add(obj, value);
	}
	else {
		return false;
	}
}

static inline size_t count(const object_t *obj) {
	ASSERT(obj);
	const collection_vtbl_t *vtbl = obj->type->collection;
	if (vtbl) {
		return vtbl->count(obj);
	}
	else {
		return false;
	}
}