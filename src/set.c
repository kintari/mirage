#include "set.h"
#include "debug.h"
#include "bst.h"
#include "object.h"
#include "iterator.h"

#include <stdlib.h>
#include <stddef.h>

struct set_t {
	object_t object;
	bst_node_t *bst;
	bst_node_t **nodes; // stored in insertion order
	size_t count;
	comparator_t compare;
};

bool set_iterator_done(iterator_t *iter) {
	size_t index = cast(iter->context, size_t);
	set_t *s = cast(iter->iterable, set_t *);
	return index >= s->count;
}

void *set_iterator_value(iterator_t *iter) {
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
	const iterable_vtbl_t *vtbl = obj->type->iterable;
	iterator_t *iterator = NULL;
	if (vtbl) {
		iterator = calloc(1, sizeof(iterator_t));
		iterator->iterable = obj;
		iterator->object.num_refs = 1;
		iterator->object.type = &set_iterator_type;
		iterator->context = cast(0, void *);
	}
	return iterator;
}

const iterable_vtbl_t set_iterable_vtbl = {
	.iterate = set_iterate
};

const type_t set_type = {
	.iterable = &set_iterable_vtbl
};

set_t *set_new(comparator_t compare) {
	set_t *s = calloc(1, sizeof(set_t));
	s->object.type = &set_type;
	s->object.num_refs = 1;
	s->bst = bst_node_new();
	s->compare = compare;
	return s;
}

void set_delete(set_t *s) {
	ASSERT(s);
	bst_traverse(s->bst, (bst_traverse_fn_t) bst_node_delete, NULL);
	free(s->nodes);
	free(s);
}

bool set_add(set_t *s, void *value) {
	bst_node_t *node = bst_search(s->bst, value, s->compare);
	bool found = node->left || node->right;
	if (!found) { // do not re-add elements		
		bst_node_t **nodes = realloc(s->nodes, (s->count + 1) * sizeof(bst_node_t *));
		ASSERT(nodes);
		if (nodes) {
			node->value = value;
			node->left = bst_node_new();
			node->right = bst_node_new();
			s->nodes = nodes;
			s->nodes[s->count++] = node;
		}
	}
	return !found;
}

struct foreach_context_t {
	bst_traverse_fn_t f;
	void *f_context;
};

static void foreach_helper(bst_node_t *node, void *context) {
	struct foreach_context_t *callback = context;
	callback->f(node->value, callback->f_context);
}

void set_foreach(set_t *s, void (*f)(void *value, void *context), void *context) {
	bst_traverse(s->bst, foreach_helper, &(struct foreach_context_t){ .f = f, .f_context = context });
}
