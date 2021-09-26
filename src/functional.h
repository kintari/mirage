#pragma once

#include <stdbool.h>

typedef struct object_t *(*map_fn_t)(struct object_t *);
typedef bool (*predicate_fn_t)(struct object_t *);

typedef struct functional_vtbl_t {
	struct object_t *(*map)(struct object_t *collection, map_fn_t f);
	struct object_t *(*filter)(struct object_t *collection, predicate_fn_t f);
	// need reduce as well
} functional_vtbl_t;

struct object_t *map(struct object_t *, map_fn_t);

struct object_t *filter(struct object_t *, predicate_fn_t);