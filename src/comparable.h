#pragma once

#include <stdbool.h>

typedef struct comparable_vtbl_t {
	bool (*eq)(const struct object_t *, const struct object_t *);
	bool (*neq)(const struct object_t *, const struct object_t *);
	bool (*gt)(const struct object_t *, const struct object_t *);
	bool (*gte)(const struct object_t *, const struct object_t *);
	bool (*lt)(const struct object_t *, const struct object_t *);
	bool (*lte)(const struct object_t *, const struct object_t *);
	int (*compare)(const struct object_t *, const struct object_t *);
} comparable_vtbl_t;
