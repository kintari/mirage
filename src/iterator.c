#include "iterator.h"
#include "debug.h"

#include <crtdbg.h>

#undef iterate

iterator_t *iterate_impl(object_t *obj) {
	DbgCheckHeap();
	iterator_t *iter = 0;
	if (obj->type->iterate) {
		iter = obj->type->iterate(obj);
	}
	DbgCheckHeap();
	return iter;
}

void *value(iterator_t *iter) {
	DbgCheckHeap();
	void *tmp = iter->value(iter);
	DbgCheckHeap();
	return tmp;
}

bool done(iterator_t *iter) {
	DbgCheckHeap();
	object_t *iterable = iter->iterable;
	bool b = iter->done(iter);
	DbgCheckHeap();
	//if (b) unref(iterable);
	(void) iterable;
	DbgCheckHeap();
	return b;
}

void advance(iterator_t *iter) {
	DbgCheckHeap();
	iter->advance(iter);
	DbgCheckHeap();
}
