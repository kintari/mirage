#pragma once

#include "type.h"

#include <stddef.h>

#define cast(x,t) ((t) (x))

typedef struct object_t object_t;
typedef struct type_t type_t;

struct object_t {
	const type_t *type;
	int num_refs;
};

object_t *allocate(const type_t *);

object_t *addref(object_t *obj);

object_t *unref(object_t *obj);
