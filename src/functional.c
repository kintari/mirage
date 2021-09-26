
#include "functional.h"

#include "debug.h"
#include "object.h"

object_t *map(object_t *obj, map_fn_t f) {
	ASSERT(obj);
	const type_t *type = obj->type;
	ASSERT(type);
	if (type->functional && type->functional->map) {
		// the type has explicitly implemented the functional protocol, so use that
		return type->functional->map(obj, f);
	}
	else if (type->iterable && type->collection) {
		// TODO: implement generic version
		return 0;
	}
	else {
		return 0; // FIXME: this needs to be a null / error object
	}
}

object_t *filter(object_t *obj, predicate_fn_t f) {
	(void) obj;
	(void) f;
	return 0;
}