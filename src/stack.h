#pragma once

#include <stddef.h>

typedef struct stack_t stack_t;

stack_t *stack_new();

void stack_delete(stack_t *stack);

size_t stack_depth(const stack_t *stack);

void stack_push(stack_t *stack, void *value);

void *stack_pop(stack_t *stack);