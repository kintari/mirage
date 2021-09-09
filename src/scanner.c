#include "scanner.h"
#include "debug.h"
#include "config.h"
#include "text.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#define MAX_STATES 64

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif
/*
#ifndef strdup
#define strdup _strdup
#endif
*/
struct scanner_t {
	FILE *file;
	char *filename;
	int lookahead[2];
	text_t *text;
	uint8_t state_tbl[MAX_STATES][256];
	size_t num_states;
	int reject_state, ignore_state, accept_state, start_state;
};

bool eos(scanner_t *scanner) {
	return scanner->lookahead[0] == -1;
}

static int read_char(scanner_t *s) {
	int ret = s->lookahead[0];
	if (ret != -1) {
		char ch;
		if (fread(&ch, 1, 1, s->file) != 1)
			ch = -1;
		s->lookahead[0] = s->lookahead[1];
		s->lookahead[1] = ch;
	}
	if (0) {
		char buf[3];
		TRACE("read_char() -> '%s'\n", escape_char(buf, ret));
	}
	return ret;
}

static int new_state(scanner_t *scanner) {	
	int state = (int) scanner->num_states++;
	for (int i = 0; i < 256; i++)
		scanner->state_tbl[state][i] = (uint8_t) scanner->accept_state;
	return state;
}

static void transition(scanner_t *scanner, int from_state, const char *values, int to_state) {
	for (const uint8_t *pch = (const uint8_t *) values; *pch != 0; pch++)
		scanner->state_tbl[from_state][*pch] = (uint8_t) to_state;
}

scanner_t *scanner_new(FILE *file, const char *filename) {
	scanner_t *scanner = calloc(1,sizeof(scanner_t));
	if (scanner) {
		scanner->file = file;
		scanner->filename = strdup(filename);
		scanner->text = text_new();

		static const char alpha_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		static const char digit_chars[] = "0123456789";
		static const char space_chars[] = " \r\n\t";
		static const char misc_chars[] = "(){}[],.;:+-*/";

		scanner->reject_state = new_state(scanner);
		scanner->accept_state = new_state(scanner);
		scanner->start_state = new_state(scanner);
		scanner->ignore_state = new_state(scanner);

		for (size_t i = 0; i < 256; i++) // default transition out of start is to reject
			scanner->state_tbl[scanner->start_state][i] = (uint8_t) scanner->reject_state;

		int drop_space_state = new_state(scanner);
		for (int i  = 0; i < 256; i++) // reading anything other than a space takes us back to start
			scanner->state_tbl[drop_space_state][i] = (uint8_t) scanner->ignore_state;
		transition(scanner, drop_space_state, space_chars, drop_space_state);

		int drop_line_state = new_state(scanner);
		for (int i = 0; i < 256; i++)
			scanner->state_tbl[drop_line_state][i] = (uint8_t) drop_line_state;
		scanner->state_tbl[drop_line_state]['\n'] = (uint8_t) scanner->accept_state;

		int scan_word_state = new_state(scanner);
		transition(scanner, scan_word_state, alpha_chars, scan_word_state);
		transition(scanner, scan_word_state, digit_chars, scan_word_state);
		transition(scanner, scan_word_state, "_", scan_word_state);

		int scan_integer_state = new_state(scanner);
		transition(scanner, scan_integer_state, digit_chars, scan_integer_state);
		int scan_frac_state = new_state(scanner);
		transition(scanner, scan_integer_state, ".", scan_frac_state);
		transition(scanner, scan_frac_state, digit_chars, scan_frac_state);

		const char *opers[] = {
			"||", "|=", "&&", "=="
		};

		for (int i = 0; i < _countof(opers); i++) {
			int state = scanner->start_state;
			for (const uint8_t *pch = (const uint8_t *) opers[i]; *pch != 0; pch++) {
				uint8_t *tmp = &scanner->state_tbl[state][*pch];
				int next_state = *tmp < scanner->start_state ? new_state(scanner) : *tmp;
				scanner->state_tbl[state][*pch] = (uint8_t) next_state;
				state = next_state;
			}
		}

		transition(scanner, scanner->start_state, space_chars, drop_space_state);
		transition(scanner, scanner->start_state, alpha_chars, scan_word_state);
		transition(scanner, scanner->start_state, "_", scan_word_state);
		transition(scanner, scanner->start_state, digit_chars, scan_integer_state);

		int scan_misc_state = new_state(scanner);
		transition(scanner, scanner->start_state, misc_chars, scan_misc_state);

		read_char(scanner);
		read_char(scanner);
	}
	return scanner;
}

void scanner_delete(scanner_t *scanner) {
	text_delete(scanner->text);
	free(scanner->filename);
	free(scanner);
}

static bool peek(scanner_t *scanner, int *pch) {
	int ch = scanner->lookahead[0];
	return ch != -1 ? (*pch = ch), true : false;
}

bool fsm_done(scanner_t *scanner, int state) {
	return state == scanner->accept_state || state == scanner->reject_state || state == scanner->ignore_state;
}

uint8_t scanner_fsm(scanner_t *s) {
	int state = s->start_state;
	int ch;
	while (peek(s, &ch) && !fsm_done(s, state = s->state_tbl[state][ch])) {
		ASSERT(state != s->start_state);
		ASSERT(state <= s->num_states);
		read_char(s);
		text_append(s->text, ch);
		//TRACE("fsm: ch='%c', state=%d\n", ch, state);
	}
	if (eos(s)) {
		// HACK: simulate a null character at end of stream to force transition into final state
		state = s->state_tbl[state][0];
	}
	return (uint8_t) state;
}

bool scanner_next(scanner_t *s, token_t *t) {

	uint8_t end_state;
	do {
		text_clear(s->text);
		end_state = scanner_fsm(s);
	} while (!eos(s) && end_state == s->ignore_state);

	if (end_state == s->accept_state) {
		t->text = text_move(s->text);
	}

	return end_state == s->accept_state;
}
