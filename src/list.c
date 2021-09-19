

/*
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "debug.h"

#define iterate(seq,var) (_Generic(seq), list_t: list_iterate)

#define foreach(seq,var_name,action) (_Generic(seq), list_t: list_foreach)

typedef struct iterator_t {
	void *container, *state;
	bool (*more)(const struct iterator_t *);
	const void *(*value)(const struct iterator_t *);
	void (*advance)(struct iterator_t *);
} iterator_t;

typedef struct array_t {
	size_t len;
} array_t;

#define array_from(...) array_new(countof(__VA_LIST__), (const void **) { __VA_LIST__ })

array_t *array_new(int len, const void **pelem) {
	array_t *array = NULL;
	if (len >= 0) {
		size_t size = sizeof(array_t) + len * sizeof(void *);
		array = malloc(size);
		array->len = len;
		void **tbl = (void **) &array[1];
		for (int i = 0; i < len; i++)
			tbl[i] = (void *) pelem[i];
	}
	return array;
}

void array_delete(array_t *array) {
	free(array);
}

void *array_get(array_t *array, int index) {
	ASSERT(array);
	ASSERT(index >= 0);
	ASSERT(index < array->len);
	void **tbl = (void **) &array[1];
	return tbl[index];
}

typedef struct list_node_t {
	const void *value;
	struct list_node_t *prev, *next;
} list_node_t;

typedef struct list_t {
	list_node_t *head, *tail;
	size_t len;
} list_t;

list_t *list_new() {
	list_t *list = malloc(sizeof(list_t));
	list->head = malloc(sizeof(list_node_t));
	list->tail = malloc(sizeof(list_node_t));
	list->head->prev = NULL;
	list->head->next = list->tail;
	list->tail->prev = list->head;
	list->tail->next = NULL;
	list->len = 0;
	return list;
}

static bool list_iter_more(const iterator_t *iter) {
	return iter->state != ((list_t *) iter->container)->tail;
}

static const void *list_iter_value(const iterator_t *iter) {
	return ((list_node_t *) iter->state)->value;
}

static void list_iter_advance(iterator_t *iter) {
	list_node_t *node = iter->state;
	iter->state = node->next;
}

iterator_t list_iterate(list_t *list) {
	return (iterator_t){
		.container = list,
			.state = list->head->next,
			.more = list_iter_more,
			.value = list_iter_value,
			.advance = list_iter_advance
	};
}

void list_append(list_t *list, const void *value) {
	list_node_t *node = malloc(sizeof(list_node_t));
	node->prev = list->tail->prev;
	node->next = list->tail;
	node->prev->next = node;
	node->next->prev = node;
	node->value = value;
	list->len++;
}

void list_foreach(list_t *list, void (*fn)(const void *)) {
	for (iterator_t iter = list_iterate(list); iter.more(&iter); iter.advance(&iter))
		fn(iter.value(&iter));
}

list_t *list_map(list_t *list, const void *(*fn)(const void *)) {
	list_t *result = NULL;
	if (result) {
		result = list_new();
		list_node_t *node = list->head->next;
		while (node != list->tail) {
			const void *value = fn(node->value);
			list_append(result, value);
			node = node->next;
		}
	}
	return result;
}

void list_delete(list_t *list) {
	ASSERT(list);
	if (list) {
		list_node_t *node = list->head->next;
		while (node != list->tail) {
			list_node_t *next = node->next;
			free(node);
			node = next;
		}
		free(list->head);
		free(list->tail);
		free(list);
	}
}
*/