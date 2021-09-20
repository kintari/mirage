#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct object_t object_t;

typedef struct list_node_t list_node_t;

typedef struct type_t {
	void (*free)(object_t *);
	struct iterator_t *(*iterate)(object_t *);
} type_t;

typedef struct object_t {
	const type_t *type;
	int num_refs;
} object_t;

typedef struct iterator_t iterator_t;

typedef struct iterator_t {
	bool (*done)(iterator_t *);
	void (*advance)(iterator_t *);
	void *(*value)(iterator_t *);
	void *iterable;
	void *context;
} iterator_t;

#define iterate(x) ((x)->object.type->iterate(&(x)->object))

/*
static inline iterator_t *iterate(object_t *obj) {
	return obj->type->iterate ? obj->type->iterate(obj) : NULL;
}
*/



struct list_node_t {
	void *value;
	struct list_node_t *prev, *next;
};

typedef struct list_t {
	object_t object;
	list_node_t *head, *tail;
	size_t count;
} list_t;

static inline void *value(iterator_t *iter) {
	list_node_t *node = iter->context;
	return node->value;
}

static inline bool done(iterator_t *iter) {
	bool b = iter->done(iter);
	if (b) free(iter);
	return b;
}

static inline void advance(iterator_t *iter) {
	iter->advance(iter);
}

list_t *list_new();

void list_delete(list_t **listp);

list_node_t *list_begin(list_t *list);

list_node_t *list_end(list_t *list);

void list_insert(list_t *list, list_node_t *pos, void *value);

void list_append(list_t *list, void *value);

void *list_remove(list_t *list, list_node_t *node);

bool list_contains(list_t *list, void *value, int (*compare)(void *, void *));

