
#include "object.h"
#include "debug.h"
#include "allocator.h"

#include <stdlib.h>

object_t *allocate(const type_t *type) {
	ASSERT(type);
	object_t *obj = 0;
	if (type) {
		if (type->allocator) {
			obj = type->allocator->allocate(type);
		}
		else if (type->size) {
			obj = calloc(1, type->size);
		}
		else {
			ASSERT("cannot allocate object" && 0);
		}
		if (obj) {
			obj->type = type;
			obj->num_refs = 1;
		}
	}
	return obj;
}

object_t *addref(object_t *obj) {
	if (obj) obj->num_refs++;
	return obj;
}

object_t *unref(object_t *obj) {
	if (obj) {
		if (--obj->num_refs == 0) {
			const type_t *type = obj->type;
			if (obj->type->destroy)
				obj->type->destroy(obj);
			if (type->allocator) {
				type->allocator->deallocate(obj);
			}
			else {
				free(obj);
			}
			return NULL;
		}
	}
	return obj;
}
