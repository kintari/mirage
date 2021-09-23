#include "bst.h"
#include "debug.h"

#include <stdlib.h>

bst_node_t *bst_node_new() {
	return calloc(1, sizeof(bst_node_t));
}

void bst_node_delete(bst_node_t *node) {
	ASSERT(node);
	free(node);
}

void bst_free(bst_node_t *node) {
	if (node) {
		bst_free(node->left);
		bst_free(node->right);
		free(node);
	}
}

void bst_traverse(bst_node_t *node, bst_traverse_fn_t f, void *context) {
	if (node->value) {
		bst_node_t *left = node->left, *right = node->right;
		bst_traverse(left, f, context);
		f(node, context);
		bst_traverse(right, f, context);
	}
}

bst_node_t *bst_search(bst_node_t *node, const void *value, int (*compare)(const void *, const void *)) {
	int cmp;
	while (node->value && ((cmp = compare(value, node->value)) != 0))
		node = (cmp == -1) ? node->left : node->right;
	return node;
}
