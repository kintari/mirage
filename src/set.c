#include "set.h"
#include "debug.h"
#include "bst.h"
#include "object.h"
#include "iterator.h"
#include "collection.h"
#include "comparable.h"
#include "functional.h"

#include <stdlib.h>
#include <stddef.h>

struct set_t {
	object_t object;
	bst_node_t *bst;
	bst_node_t **nodes; // stored in insertion order
	size_t count;
};

bool set_collection_add(object_t *obj, object_t *value) {
	return set_add(cast(obj, set_t *), value);
}

size_t set_collection_count(const object_t *obj) {
	return cast(obj, const set_t *)->count;
}

const collection_vtbl_t set_collection_vtbl = {
	.add = set_collection_add,
	.count = set_collection_count
};

bool set_iterator_done(iterator_t *iter) {
	size_t index = cast(iter->context, size_t);
	set_t *s = cast(iter->iterable, set_t *);
	return index >= s->count;
}

object_t *set_iterator_value(iterator_t *iter) {
	size_t index = cast(iter->context, size_t);
	set_t *s = cast(iter->iterable, set_t *);
	ASSERT(index < s->count);
	return s->nodes[index]->value;
}

void set_iterator_advance(iterator_t *iter) {
	size_t *indexp = cast(&iter->context, size_t *);
	set_t *s = cast(iter->iterable, set_t *);
	if (*indexp < s->count) *indexp += 1;
}

const iterator_vtbl_t set_iterator_vtbl = {
	.done = set_iterator_done,
	.value = set_iterator_value,
	.advance = set_iterator_advance,
};

const type_t set_iterator_type = {
	.iterator = &set_iterator_vtbl
};

iterator_t *set_iterate(object_t *obj) {
	ASSERT(obj);
	iterator_t *iterator = NULL;
	const iterable_vtbl_t *vtbl = obj->type->iterable;
	if (vtbl) {
		iterator = calloc(1, sizeof(iterator_t));
		iterator->object.num_refs = 1;
		iterator->object.type = &set_iterator_type;
		iterator->iterable = obj;
	}
	return iterator;
}

const iterable_vtbl_t set_iterable_vtbl = {
	.iterate = set_iterate
};

set_t *set_new() {
	set_t *s = (set_t *) allocate(&set_type);
	s->bst = bst_node_new();
	return s;
}

void set_destroy(set_t *s) {
	ASSERT(s);
	for (size_t i = 0; i < s->count; i++) {
		bst_node_t *node = s->nodes[i];
		unref(node->value);
		bst_node_delete(node);
	}
	free(s->nodes);
}

bool set_add(set_t *s, object_t *value) {
	bst_node_t *node = bst_search(s->bst, value, compare);
	bool found = node->left || node->right;
	if (!found) { // do not re-add elements
		bst_node_t **nodes = realloc(s->nodes, (s->count + 1) * sizeof(bst_node_t *));
		ASSERT(nodes);
		if (nodes) {
			node->value = addref(value);
			node->left = bst_node_new();
			node->right = bst_node_new();
			s->nodes = nodes;
			s->nodes[s->count++] = node;
		}
	}
	return !found;
}

const type_t set_type = {
	.name = "set",
	.size = sizeof(set_t),
	.destroy = (destructor_t) set_destroy,
	.collection = &set_collection_vtbl,
	.iterable = &set_iterable_vtbl
};
