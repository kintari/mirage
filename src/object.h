#pragma once

#define cast(x,t) ((t) (x))

typedef struct object_t object_t;
typedef struct type_t type_t;

typedef void (*destructor_t)(object_t *);

struct type_t {
	destructor_t destroy;
	const struct collection_vtbl_t *collection;
	const struct functional_vtbl_t *functional;
	const struct iterable_vtbl_t *iterable;
	const struct iterator_vtbl_t *iterator;
};

struct object_t {
	const type_t *type;
	int num_refs;
};

static inline object_t *addref_impl(object_t *obj) {
	if (obj) ++obj->num_refs;
	return obj;
}

static inline object_t *unref_impl(object_t *obj) {
	if (obj) {
		if (--obj->num_refs == 0) {
			if (obj->type->destroy)
				obj->type->destroy(obj);
			return 0;
		}
	}
	return obj;
}

#define addref(x) addref_impl((object_t *) x)
#define unref(x) unref_impl((object_t *) (x))