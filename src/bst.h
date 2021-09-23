#pragma once

typedef struct bst_node_t bst_node_t;

struct bst_node_t {
	void *value;
	struct bst_node_t *left, *right;
};

bst_node_t *bst_node_new();

void bst_node_delete(bst_node_t *node);

typedef void (*bst_traverse_fn_t)(bst_node_t *node, void *context);

void bst_traverse(bst_node_t *node, bst_traverse_fn_t f, void *context);

bst_node_t *bst_search(bst_node_t *node, const void *value, int (*compare)(const void *, const void *));