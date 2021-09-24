#include "config.h"
#include "parser.h"
#include "debug.h"
#include "list.h"
#include "stack.h"
#include "set.h"
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

static object_t *closure(list_t *kernel) {

	list_t *lr_items = list_new();
	object_t *result = (object_t *) set_new(lr_item_cmp);

	foreach ((object_t *) kernel, iter)
		list_insert(lr_items, list_end(lr_items), value(iter));

	while (lr_items->count > 0) {
		lr_item_t *item = list_remove(lr_items, lr_items->head->next);		
		add(result, item);
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
		object_t *state = closure(kernel);
		list_delete(kernel);

		TRACE("state %d:\n", num_states);
		foreach(state, iter) {
			print_item(value(iter));
			TRACE("\n");
		}
		num_states++;

		// collect all symbols which immediately follow a dot
		object_t *symbols = (object_t *) set_new(strcmp);
		foreach(state, iter) {
			lr_item_t *item = value(iter);
			// only consider items with something after the dot
			const char *symbol = item->rhs[item->dot+1];
			if (symbol) {
				char *symbol_copy = strdup(symbol);
				if (!add(symbols, symbol_copy))
					free(symbol_copy);
			}
		}

		if (count(symbols) > 0) { 
			TRACE("symbols leading to other states:");
			foreach((object_t *) symbols, iter)
				TRACE(" %s", value(iter));
			TRACE("\n");
		}

		// for each symbol in symbols, build the kernel of the state that it leads to
		foreach(symbols, iter) {
			char *symbol = value(iter);
			if (symbol) {
				list_t *new_kernel = list_new();
				//for (list_node_t *item_node = state->head->next; item_node != state->tail; item_node = item_node->next) {
				foreach (state, item_iter) {
					lr_item_t *item = value(item_iter);
					if (item->dot < item->rhs_len && strcmp(item->rhs[item->dot+1], symbol) == 0)
						list_append(new_kernel, lr_item_new(item->rule, item->dot + 1));
				}
				list_count(new_kernel) > 0 ? stack_push(stack, new_kernel) : list_delete(new_kernel);
			}
		}

		TRACE("\n");

		//for (list_node_t *node = state->head->next; node != state->tail; node = node->next)
		//	lr_item_delete(node->value);
		
		//foreach((object_t *) symbols, iter)
		//	free(value(iter));
		unref(symbols);
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