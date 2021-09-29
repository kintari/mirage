#include "comparable.h"
#include "object.h"
#include "debug.h"

#include <stdlib.h>

int compare(const object_t *x, const object_t *y) {
	ASSERT(x);
	ASSERT(x);
	ASSERT(x->type == y->type);
	const type_t *type = x->type;
	ASSERT(type->comparable);
	if (type->comparable->compare) {
		return type->comparable->compare(x, y);
	}
	else if (type->comparable->gt && type->comparable->lt && type->comparable->eq) {
		if (type->comparable->gt(x, y)) {
			return 1;
		}
		if (type->comparable->lt(x, y)) {
			return -1;
		}
		else {
			return type->comparable->eq(x, y);
		}
	}
	else {
		ASSERT("cannot compare objects" && false);
		abort();
	}
}
