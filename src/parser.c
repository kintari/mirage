#include "config.h"
#include "parser.h"
#include "debug.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct iterator_t {
	bool (*done)(const struct iterator_t *);
	void *(*value)(struct iterator_t *);
	void (*advance)(struct iterator_t *);
	void *collection;
} iterator_t;

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

/*
typedef struct set_iter_t {
	iterator_t base;
	bst_node_t **nodes;
	size_t num_nodes;
} set_iter_t;

void *set_iter_value(set_iter_t *iter) {

}

bool set_iter_done(const set_iter_t *iter) {
	return true;
}

void set_iter_advance(set_iter_t *s) {

}

iterator_t *set_iterate(set_t *s) {
	set_iter_t *iter = malloc(sizeof(set_iter_t));
	iter->base.advance = set_iter_advance;
	iter->base.done = set_iter_done;
	iter->base.value = set_iter_value;
	if (s->count != 0) {
		iter->nodes = calloc(1, sizeof(bst_node_t *));
		iter->nodes[0] = s->root;
		iter->num_nodes = 1;
	}
	else {
		iter->nodes = NULL;
		iter->num_nodes = 0;
	}
}
*/


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


typedef struct list_node_t {
	void *value;
	struct list_node_t *prev, *next;
} list_node_t;

typedef struct list_t {
	list_node_t *head, *tail;
	size_t count;
} list_t;

list_t *list_new() {
	list_t *list = calloc(1, sizeof(list_t));
	list->head = calloc(1, sizeof(list_node_t));
	list->tail = calloc(1, sizeof(list_node_t));
	list->head->next = list->tail;
	list->tail->prev = list->head;
	return list;
}

void list_delete(list_t **listp) {
	ASSERT(listp);
	ASSERT(*listp);
	list_t *list = *listp;
	list_node_t *node = list->head;
	while (node) {
		list_node_t *next = node->next;
		free(node);
		node = next;
	}
	free(list);
	*listp = NULL;
}

list_node_t *list_begin(list_t *list) {
	return list->head->next;
}

list_node_t *list_end(list_t *list) {
	return list->tail;
}

void list_insert(list_t *list, list_node_t *pos, void *value) {
	// new node is inserted before 'pos'
	ASSERT(pos);
	ASSERT(pos->prev);
	list_node_t *node = calloc(1, sizeof(list_node_t));
	node->value = value;
	node->prev = pos->prev;
	node->next = pos;
	node->prev->next = node;
	node->next->prev = node;
	list->count++;
}

void list_append(list_t *list, void *value) {
	list_insert(list, list_end(list), value);
}

void *list_remove(list_t *list, list_node_t *node) {
	ASSERT(list);
	ASSERT(node);
	ASSERT(node->prev);
	ASSERT(node->next);
	node->prev->next = node->next;
	node->next->prev = node->prev;
	void *result = node->value;
	free(node);
	list->count--;
	return result;
}

static int trivial_compare(void *x, void *y) {
	if (x > y) return  1;
	if (x < y) return -1;
	return 0;
}

bool list_contains(list_t *list, void *value, int (*compare)(void *, void *)) {
	list_node_t *node = list->head->next;
	if (compare == NULL) compare = trivial_compare;
	while (node != list->tail) {
		if (compare(value, node->value) == 0)
			return true;
		node = node->next;
	}
	return false;
}

typedef struct rule_t {
	const char *lhs;
	const char **rhs;
} rule_t;

static rule_t grammar[] = {
	{ "S", (const char *[]) { "FUNCTION", "$", 0 } },
	{ "FUNCTION", (const char *[]){ "function", "IDENTIFIER", "(", "PARAMETER", ")", ":", "TYPE-SPECIFIER", "STATEMENT-BLOCK", 0 } },
	{ "PARAMETER", (const char *[]){ "IDENTIFIER", ":", "TYPE-SPECIFIER", 0 } },
	{ "STATEMENT-BLOCK", (const char *[]){ "{", "}", 0 } },
	{ "TYPE-SPECIFIER", (const char *[]){ "u32", 0 } },
	{ "TYPE-SPECIFIER", (const char *[]){ "IDENTIFIER", 0 } },
	{ 0 }
};

typedef struct lr_item_t {
	const rule_t *rule;
	char *lhs;
	char **rhs;
	size_t rhs_len;
	int dot;
} lr_item_t;

static lr_item_t *lr_item_new(const rule_t *rule, int dot) {
	lr_item_t *item = malloc(sizeof(lr_item_t));
	item->rule = rule;
	item->lhs = strdup(rule->lhs);
	item->rhs_len = 0;
	for (const char **pstr = rule->rhs; *pstr != 0; pstr++)
		item->rhs_len++;
	item->rhs_len++; // for the dot
	item->rhs = calloc(item->rhs_len + 1 /* for the trailing null */, sizeof(void *));
	for (int i = 0; i < dot; i++)
		item->rhs[i] = strdup(rule->rhs[i]);
	item->rhs[dot] = strdup(".");
	for (int i = dot; i + 1 < item->rhs_len; i++)
		item->rhs[i+1] = strdup(rule->rhs[i]);
	item->dot = dot;
	return item;
}

static void lr_item_delete(lr_item_t *item) {
	if (item) {
		free(item->lhs);
		for (size_t i = 0; i < item->rhs_len; i++)
			free(item->rhs[i]);
		free(item->rhs);
		free(item->lhs);
	}
}

/*
static lr_item_t *lr_item_clone(const lr_item_t *item) {
	lr_item_t *clone = malloc(sizeof(lr_item_t));
	*clone = *item;
	return clone;
}
*/

static int lr_item_cmp(const lr_item_t *x, const lr_item_t *y) {
	int cmp = strcmp(x->lhs, y->lhs);
	if (cmp) return cmp;
	char **px = x->rhs, **py = y->rhs;
	while (*px && *py) {
		cmp = strcmp(*px, *py);
		if (cmp) return cmp;
		++px;
		++py;
	}
	if (*px == 0) return -1;
	if (*py == 0) return 1;
	return 0;
}

void print_item(const lr_item_t *item) {
	TRACE("%s ->", item->lhs);
	for (size_t i = 0; i < item->rhs_len; i++)
		TRACE(" %s", item->rhs[i]);
}


static list_t *closure(list_t *kernel) {
	ASSERT(kernel);
	
	list_t *lr_items = list_new();
	list_t *result = list_new();

	for (list_node_t *node = list_begin(kernel); node != list_end(kernel); node = node->next)
		list_insert(lr_items, list_end(lr_items), node->value);

	while (lr_items->count > 0) {
		lr_item_t *item = list_remove(lr_items, lr_items->head->next);
		if (!list_contains(result, item, (int (*)(void *, void *)) lr_item_cmp))
			list_insert(result, list_end(result), item);
		const char *next_symbol = item->rhs[item->dot+1];
		if (next_symbol) {
			// find all rules with 'symbol' as their left hand side
			for (const rule_t *rule = grammar; rule->lhs != NULL; rule++) {
				if (strcmp(rule->lhs, next_symbol) == 0) {
					// add a new lr item to the queue
					lr_item_t *new_item = lr_item_new(rule, 0);
					list_insert(lr_items, list_end(lr_items), new_item);
				}
			}
		}
	}

	return result;
}

struct parser_t {
	int state;
};

void print_itemset(list_t *item_set) {
	for (list_node_t *node = list_begin(item_set); node != list_end(item_set); node = node->next) {
		print_item(node->value);
		TRACE("\n");
	}
}

parser_t *parser_new() {


	// create the start item set, containing a single item corresponding to the start rule S -> whatever
	list_t *start_state = list_new();
	list_insert(start_state, list_end(start_state), lr_item_new(&grammar[0], 0));

	// this stack stores item sets to be processed
	stack_t *stack = stack_new();
	stack_push(stack, start_state);

	size_t num_states = 0;
	while (stack_depth(stack) > 0) {

		// get the next item set and compute its closure
		list_t *kernel = stack_pop(stack);
		list_t *state = closure(kernel);
		list_delete(&kernel);

		TRACE("state %d:\n", num_states);
		print_itemset(state);
		num_states++;

		// collect all symbols which immediately follow a dot
		list_t *symbols = list_new();
		for (list_node_t *node = state->head->next; node != state->tail; node = node->next) {
			lr_item_t *item = node->value;
			// only consider items with something after the dot
			if (item->dot < item->rhs_len) {
				char *symbol = item->rhs[item->dot+1];
				if (symbol && !list_contains(symbols, symbol, (int (*)(void *, void *)) strcmp)) {
					list_append(symbols, strdup(symbol));
				}
			}
		}

		TRACE("symbols leading to other states:");
		for (list_node_t *node = symbols->head->next; node != symbols->tail; node = node->next)
			TRACE(" %s", node->value);
		TRACE("\n");

		// for each symbol in symbols, build the kernel of the state that it leads to
		for (list_node_t *node = symbols->head->next; node != symbols->tail; node = node->next) {
			char *symbol = node->value;
			if (symbol) {
				list_t *new_kernel = list_new();
				for (list_node_t *item_node = state->head->next; item_node != state->tail; item_node = item_node->next) {
					lr_item_t *item = item_node->value;
					if (item->dot < item->rhs_len && strcmp(item->rhs[item->dot+1], symbol) == 0)
						list_append(new_kernel, lr_item_new(item->rule, item->dot + 1));
				}
				stack_push(stack, new_kernel);
			}
		}

		TRACE("\n");
		
		if (0) {
		for (list_node_t *node = state->head->next; node != state->tail; node = node->next)
			lr_item_delete(node->value);
		}
		
		//for (list_node_t *node = symbols->head->next; node != symbols->tail; node = node->next)
		//	free(node->value);
		list_delete(&symbols);
	}

	parser_t *parser = calloc(1,sizeof(parser_t));
	return parser;
}

void parser_delete(parser_t *parser) {
	free(parser);
}

int parser_advance(parser_t *parser, const token_t *token) {
	(void) parser;
	(void) token;
	return 0;
}