#include "config.h"
#include "parser.h"
#include "debug.h"
#include "list.h"
#include "stack.h"
#include "iterator.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

	DbgCheckHeap();

	for (iterator_t *iter = iterate(kernel); !done(iter); advance(iter)) {
		list_insert(lr_items, list_end(lr_items), value(iter));
	}

	/*
	for (list_node_t *node = list_begin(kernel); node != list_end(kernel); node = node->next) {
		list_insert(lr_items, list_end(lr_items), node->value);
	}
	*/

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
		list_delete(kernel);
		DbgCheckHeap();

		TRACE("state %d:\n", num_states);
		print_itemset(state);
		num_states++;
		DbgCheckHeap();

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
				DbgCheckHeap();
			}
		}

		DbgCheckHeap();
		TRACE("symbols leading to other states:");
		for (list_node_t *node = symbols->head->next; node != symbols->tail; node = node->next)
			TRACE(" %s", node->value);
		TRACE("\n");
		DbgCheckHeap();

		// for each symbol in symbols, build the kernel of the state that it leads to
		for (list_node_t *node = symbols->head->next; node != symbols->tail; node = node->next) {
			char *symbol = node->value;
			if (symbol) {
				DbgCheckHeap();
				list_t *new_kernel = list_new();
				DbgCheckHeap();
				for (list_node_t *item_node = state->head->next; item_node != state->tail; item_node = item_node->next) {
					lr_item_t *item = item_node->value;
					if (item->dot < item->rhs_len && strcmp(item->rhs[item->dot+1], symbol) == 0)
						list_append(new_kernel, lr_item_new(item->rule, item->dot + 1));
					DbgCheckHeap();
				}
				stack_push(stack, new_kernel);
				DbgCheckHeap();
			}
		}

		TRACE("\n");
		
		if (0) {
		for (list_node_t *node = state->head->next; node != state->tail; node = node->next)
			lr_item_delete(node->value);
			DbgCheckHeap();
		}
		
		//for (list_node_t *node = symbols->head->next; node != symbols->tail; node = node->next)
		//	free(node->value);
		list_delete(symbols);
		DbgCheckHeap();
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