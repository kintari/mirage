#pragma once

typedef struct allocator_vtbl_t {
	struct object_t *(*allocate)(const struct type_t *);
	void (*deallocate)(struct object_t *);
} allocator_vtbl_t;
