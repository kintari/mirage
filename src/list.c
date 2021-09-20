#include "list.h"
#include "debug.h"

#include <stdlib.h>

void *list_iter_value(iterator_t *iter) {
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

iterator_t *list_iterate(object_t *obj) {
	list_t *list = (list_t *) obj;
	iterator_t *iter = calloc(1,sizeof(iter));
	iter->value = list_iter_value;
	iter->done = list_iter_done;
	iter->advance = list_iter_advance;
	iter->iterable = obj;
	iter->context = list->head->next;
	return iter;
}

const type_t list_type = {
	.iterate = list_iterate
};

list_t *list_new() {
	list_t *list = calloc(1, sizeof(list_t));
	list->object.type = &list_type;
	list->object.num_refs = 1;
	list->head = calloc(1, sizeof(list_node_t));
	list->tail = calloc(1, sizeof(list_node_t));
	list->head->next = list->tail;
	list->tail->prev = list->head;
	return list;
}

void list_delete(list_t **listp) {
	ASSERT(listp);
	ASSERT(*listp);
	list_t *list = *listp;
	list_node_t *node = list->head;
	while (node) {
		list_node_t *next = node->next;
		free(node);
		node = next;
	}
	free(list);
	*listp = NULL;
}

list_node_t *list_begin(list_t *list) {
	return list->head->next;
}

list_node_t *list_end(list_t *list) {
	return list->tail;
}

void list_insert(list_t *list, list_node_t *pos, void *value) {
	// new node is inserted before 'pos'
	ASSERT(pos);
	ASSERT(pos->prev);
	list_node_t *node = calloc(1, sizeof(list_node_t));
	node->value = value;
	node->prev = pos->prev;
	node->next = pos;
	node->prev->next = node;
	node->next->prev = node;
	list->count++;
}

void list_append(list_t *list, void *value) {
	list_insert(list, list_end(list), value);
}

void *list_remove(list_t *list, list_node_t *node) {
	ASSERT(list);
	ASSERT(node);
	ASSERT(node->prev);
	ASSERT(node->next);
	node->prev->next = node->next;
	node->next->prev = node->prev;
	void *result = node->value;
	free(node);
	list->count--;
	return result;
}

static int trivial_compare(void *x, void *y) {
	if (x > y) return  1;
	if (x < y) return -1;
	return 0;
}

bool list_contains(list_t *list, void *value, int (*compare)(void *, void *)) {
	list_node_t *node = list->head->next;
	if (compare == NULL) compare = trivial_compare;
	while (node != list->tail) {
		if (compare(value, node->value) == 0)
			return true;
		node = node->next;
	}
	return false;
}