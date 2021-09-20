#include "set.h"
#include "debug.h"

#include <stdlib.h>

typedef struct bst_node_t {
	void *value;
	struct bst_node_t *left, *right;
} bst_node_t;

bst_node_t *bst_node_new() {
	return calloc(1, sizeof(bst_node_t));
}

void bst_free(bst_node_t *node) {
	if (node) {
		bst_free(node->left);
		bst_free(node->right);
		free(node);
	}
}

void bst_traverse(bst_node_t *node, void (*f)(void *value, void *context), void *context) {
	if (node->value) {
		bst_traverse(node->left, f, context);
		f(node->value, context);
		bst_traverse(node->right, f, context);
	}
}

typedef struct set_t {
	bst_node_t *root;
	size_t count;
	int (*compare)(const void *, const void *);
} set_t;

set_t *set_new(int (*compare)(const void *, const void *)) {
	set_t *s = malloc(sizeof(set_t));
	s->root = bst_node_new();
	s->count = 0;
	s->compare = compare;
	return s;
}

void set_delete(set_t **setp) {
	ASSERT(setp);
	bst_free((*setp)->root);
	free(*setp);
	*setp = NULL;
}

static bst_node_t *set_search(set_t *s, const void *value) {
	bst_node_t *node = s->root;
	int cmp;
	while (node->value && ((cmp = s->compare(value, node->value)) != 0))
		node = (cmp == -1) ? node->left : node->right;
	return node;
}

void set_add(set_t *s, void *value) {
	bst_node_t *node = set_search(s, value);
	if (node->value == NULL) { // do not re-add elements
		node->value = value;
		node->left = bst_node_new();
		node->right = bst_node_new();
		s->count++;
	}
}

void set_foreach(set_t *s, void (*f)(void *value, void *context), void *context) {
	bst_traverse(s->root, f, context);
}
