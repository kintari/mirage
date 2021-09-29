/*
#include "stack.h"
#include "type.h"
#include "object.h"
#include "debug.h"

#include <stdlib.h>
#include <stddef.h>

typedef struct stack_node_t {
	object_t *value;
	struct stack_node_t *next;
} stack_node_t;

typedef struct stack_t {
	stack_node_t *top;
	size_t depth;
} stack_t;

void stack_destroy(stack_t *stack) {
	stack_node_t *node = stack->top;
	while (node != NULL) {
		stack_node_t *next = node->next;
		unref(node->value);
		free(node);
		node = next;
	}
}

size_t stack_depth(const stack_t *stack) {
	ASSERT(stack);
	return stack->depth;
}

void stack_push(stack_t *stack, object_t *value) {
	ASSERT(stack);
	stack_node_t *node = malloc(sizeof(stack_node_t));
	node->value = addref(value);
	node->next = stack->top;
	stack->top = node;
	stack->depth++;
}

object_t *stack_pop(stack_t *stack) {
	ASSERT(stack->depth);
	if (stack->depth == 0)
		abort();
	stack_node_t *node = stack->top;
	stack->top = stack->top->next;
	stack->depth--;
	object_t *value = node->value;
	free(node);
	return value;
}

const type_t stack_type = {
	.name = "stack",
	.size = sizeof(stack_t),
	.destroy = stack_destroy
};
*/

static void blarg() {

}