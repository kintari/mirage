#pragma once

#include "collection.h"

#include <stdbool.h>

typedef struct set_t set_t;

set_t *set_new();

void set_delete(set_t *);

bool set_add(set_t *, void *);
