#include "stack.h"
#include "debug.h"

#include <stdlib.h>
#include <stddef.h>

typedef struct stack_node_t {
	void *value;
	struct stack_node_t *next;
} stack_node_t;

typedef struct stack_t {
	stack_node_t *top;
	size_t depth;
} stack_t;

stack_t *stack_new() {
	return calloc(1, sizeof(stack_t));
}

void stack_delete(stack_t *stack) {
	stack_node_t *node = stack->top;
	while (node != NULL) {
		stack_node_t *next = node->next;
		free(node);
		node = next;
	}
	free(stack);
}

size_t stack_depth(const stack_t *stack) {
	ASSERT(stack);
	return stack->depth;
}

void stack_push(stack_t *stack, void *value) {
	ASSERT(stack);
	stack_node_t *node = malloc(sizeof(stack_node_t));
	node->value = value;
	node->next = stack->top;
	stack->top = node;
	stack->depth++;
}

void *stack_pop(stack_t *stack) {
	ASSERT(stack->depth);
	if (stack->depth == 0)
		abort();
	stack_node_t *node = stack->top;
	stack->top = stack->top->next;
	stack->depth--;
	void *value = node->value;
	free(node);
	return value;
}
