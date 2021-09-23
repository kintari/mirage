#include "iterator.h"
#include "debug.h"
#include "object.h"

#include <crtdbg.h>

iterator_t *iterate(object_t *obj) {
	addref(obj);
	const iterable_vtbl_t *vtbl = obj->type->iterable;
	iterator_t *iter = vtbl ? vtbl->iterate(obj) : NULL;
	unref(obj);
	return iter;
}

void *value(iterator_t *iter) {
	const iterator_vtbl_t *vtbl = iter->object.type->iterator;
	return vtbl->value(iter);
}

bool done(iterator_t *iter) {
	const iterator_vtbl_t *vtbl = iter->object.type->iterator;
	//object_t *iterable = iter->iterable;
	bool b = vtbl->done(iter);
	//if (b) unref(iterable);
	return b;
}

void advance(iterator_t *iter) {
	const iterator_vtbl_t *vtbl = iter->object.type->iterator;
	vtbl->advance(iter);
	iter->ordinal++;
}
