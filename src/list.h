#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#include "object.h"
#include "collection.h"

typedef struct list_node_t list_node_t;

struct list_node_t {
	void *value;
	struct list_node_t *prev, *next;
};

typedef struct list_t {
	object_t object;
	list_node_t *head, *tail;
	size_t count;
} list_t;

list_t *list_new();

void list_delete(list_t *list);

size_t list_count(list_t *list);

list_node_t *list_begin(list_t *list);

list_node_t *list_end(list_t *list);

void list_insert(list_t *list, list_node_t *pos, void *value);

void list_append(list_t *list, void *value);

void *list_remove(list_t *list, list_node_t *node);

bool list_contains(list_t *list, const void *value, int (*compare)(void *, void *));

