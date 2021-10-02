#include "config.h"
#include "parser.h"
#include "debug.h"
#include "list.h"
#include "stack.h"
#include "set.h"
#include "iterator.h"
#include "functional.h"
#include "comparable.h"

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
	object_t object;
	const rule_t *rule;
	char *lhs;
	char **rhs;
	size_t rhs_len;
	int dot;
} lr_item_t;


static int lr_item_cmp(const lr_item_t *, const lr_item_t *);

const comparable_vtbl_t lr_item_comparable_vtbl = {
	.compare = (int (*)(const object_t *, const object_t *)) lr_item_cmp
};

const type_t lr_item_type = {
	.comparable = &lr_item_comparable_vtbl
};

static lr_item_t *lr_item_new(const rule_t *rule, int dot) {
	lr_item_t *item = malloc(sizeof(lr_item_t));
	item->object.num_refs = 1;
	item->object.type = &lr_item_type;
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
	object_t *result = (object_t *) set_new();

	foreach ((object_t *) kernel, iter)
		list_insert(lr_items, list_end(lr_items), value(iter));

	while (lr_items->count > 0) {
		lr_item_t *item = (lr_item_t *) addref(lr_items->head->next->value);
		list_remove(lr_items, lr_items->head->next);
		add(result, item);
		const char *next_symbol = item->rhs[item->dot+1];
		if (next_symbol) {
			// find all rules with 'symbol' as their left hand side
			for (const rule_t *rule = grammar; rule->lhs != NULL; rule++) {
				if (strcmp(rule->lhs, next_symbol) == 0) {
					// add a new lr item to the queue
					lr_item_t *new_item = lr_item_new(rule, 0);
					list_insert(lr_items, list_end(lr_items), (object_t *) new_item);
				}
			}
		}
	}

	return result;
}

struct parser_t {
	object_t *states;
};

parser_t *parser_new() {

	// create the start item set, containing a single item corresponding to the start rule S -> whatever
	object_t *start_state = (object_t *) list_new(&list_type);
	add(start_state, (object_t *) lr_item_new(&grammar[0], 0));

	// this stack stores item sets to be processed
	list_t *stack = list_new();
	list_append(stack, start_state);

	object_t *states = (object_t *) list_new();

	while (list_count(stack) > 0) {

		// pop the next item set
		list_t *kernel = (list_t *) addref(stack->tail->prev->value);
		list_remove(stack, stack->tail->prev);

		// compute its closure
		object_t *state = closure(kernel);
		add(states, state);

		TRACE("state %d:\n", count(states));
		foreach(state, iter) {
			print_item(cast(value(iter), lr_item_t *));
			TRACE("\n");
		}

		// collect all symbols which immediately follow a dot
		object_t *symbols = (object_t *) set_new(&set_type);
		foreach(state, iter) {
			lr_item_t *item = (lr_item_t *) value(iter);
			// only consider items with something after the dot
			char *buf = item->rhs[item->dot+1];
			text_t *symbol = text_new_from_cstr(buf);
			if (symbol) add(symbols, symbol);
		}

		if (count(symbols) > 0) { 
			TRACE("symbols leading to other states:");
			foreach(symbols, iter) {
				text_t *text = (text_t *) value(iter);
				TRACE(" %s", text_buf(text));
			}
			TRACE("\n");
		}

		// for each symbol in symbols, build the kernel of the state that it leads to
		foreach(symbols, iter) {
			text_t *symbol = (text_t *) value(iter);
			object_t *new_kernel = (object_t *) list_new();
			foreach (state, item_iter) {
				lr_item_t *item = (lr_item_t *) value(item_iter);
				const char *after = item->rhs[item->dot+1];
				if (after && strcmp(after, text_buf(symbol)) == 0)
					add(new_kernel, (object_t *) lr_item_new(item->rule, item->dot + 1));
			}
			if (count(new_kernel) > 0)
				list_append(stack, new_kernel);
			unref(new_kernel);
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