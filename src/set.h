#pragma once

#include "collection.h"

#include <stdbool.h>

typedef struct set_t set_t;

set_t *set_new();

bool set_add(set_t *, struct object_t *);

extern const type_t set_type;