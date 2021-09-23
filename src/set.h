#pragma once

#include <stdbool.h>

typedef struct set_t set_t;

typedef int (*comparator_t)(const void *, const void *);

set_t *set_new(comparator_t comparator);

void set_delete(set_t *);

bool set_add(set_t *, void *);
