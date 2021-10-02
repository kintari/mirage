#include "list.h"
#include "debug.h"
#include "iterator.h"
#include "functional.h"
#include "comparable.h"

#include <stdlib.h>

bool list_collection_add(object_t *obj, void *value) {
	list_append(cast(obj, list_t *), value);
	return true;
}

size_t list_collection_count(const object_t *obj) {
	return cast(obj, list_t *)->count;
}

const collection_vtbl_t list_collection_vtbl = {
	.add = list_collection_add,
	.count = list_collection_count
};

object_t *list_functional_map(object_t *obj, map_fn_t f) {
	ASSERT(obj);
	ASSERT(f);
	list_t *src = cast(obj, list_t *);
	list_t *result = list_new();
	for (list_node_t *node = src->head->next; node != src->tail; node = node->next)
		list_append(result, f(node->value));
	return (object_t *) result;
}

object_t *list_functional_filter(object_t *obj, predicate_fn_t f) {
	ASSERT(obj);
	ASSERT(f);
	list_t *src = cast(obj, list_t *);
	list_t *result = list_new();
	for (list_node_t *node = src->head->next; node != src->tail; node = node->next)
		if (f(node->value))
			list_append(result, node->value);
	return (object_t *) result;
}

const functional_vtbl_t list_functional_vtbl = {
	.map = list_functional_map,
	.filter = list_functional_filter
};

object_t *list_iter_value(iterator_t *iter) {
	return ((list_node_t *) iter->context)->value;
}

bool list_iter_done(iterator_t *iter) {
	list_node_t *node = (list_node_t *) iter->context;
	list_t *list = (list_t *) iter->iterable;
	return node == list->tail;
}

void list_iter_advance(iterator_t *iter) {
	list_node_t *node = (list_node_t *) iter->context;
	iter->context = node->next;
}

const iterator_vtbl_t list_iterator_vtbl = {
	.value = list_iter_value,
	.done = list_iter_done,
	.advance = list_iter_advance
};

const type_t list_iterator_type = {
	.destroy = free,
	.iterator = &list_iterator_vtbl
};

iterator_t *list_iterate(object_t *obj) {
	list_t *list = (list_t *) obj;
	iterator_t *iter = calloc(1,sizeof(iterator_t));
	iter->object.type = &list_iterator_type;
	iter->object.num_refs = 1;
	iter->iterable = addref(obj);
	iter->context = list->head->next;
	iter->ordinal = 0;
	return iter;
}

const iterable_vtbl_t list_iterable_vtbl = {
	.iterate = list_iterate
};

const type_t list_type = {
	.size = sizeof(list_t),
	.destroy = (destructor_t) list_destroy,
	.collection = &list_collection_vtbl,
	.functional = &list_functional_vtbl,
	.iterable = &list_iterable_vtbl
};

list_t *list_new() {
	list_t *list = (list_t *) allocate(&list_type);
	list->head = calloc(1, sizeof(list_node_t));
	list->tail = calloc(1, sizeof(list_node_t));
	list->head->next = list->tail;
	list->tail->prev = list->head;
	return list;
}

void list_destroy(list_t *list) {
	ASSERT(list);
	list_node_t *node = list->head;
	while (node) {
		list_node_t *next = node->next;
		unref(node->value);
		free(node);
		node = next;
	}
}

size_t list_count(list_t *list) {
	ASSERT(list);
	return list->count;
}

list_node_t *list_begin(list_t *list) {
	return list->head->next;
}

list_node_t *list_end(list_t *list) {
	return list->tail;
}

void list_insert(list_t *list, list_node_t *pos, object_t *value) {
	// new node is inserted before 'pos'
	ASSERT(pos);
	ASSERT(pos->prev);
	list_node_t *node = calloc(1, sizeof(list_node_t));
	node->value = addref(value);
	node->prev = pos->prev;
	node->next = pos;
	node->prev->next = node;
	node->next->prev = node;
	list->count++;
}

void list_append(list_t *list, object_t *value) {
	list_insert(list, list_end(list), value);
}

void list_remove(list_t *list, list_node_t *node) {
	ASSERT(list);
	ASSERT(node);
	ASSERT(node->prev);
	ASSERT(node->next);
	node->prev->next = node->next;
	node->next->prev = node->prev;
	object_t *value = node->value;
	free(node);
	unref(value);
	list->count--;
}

bool list_contains(list_t *list, object_t *value, int (*compare_fn)(object_t *, object_t *)) {
	list_node_t *node = list->head->next;
	if (compare_fn == NULL)
		compare_fn = compare;
	while (node != list->tail) {
		if (compare(value, node->value) == 0)
			return true;
		node = node->next;
	}
	return false;
}