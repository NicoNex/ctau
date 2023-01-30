#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "lexer.h"
#include "../item/item.h"

#define SETSTATE(s) ({ l->state = s; return; })
#define eof '\0'

static inline int next(struct lexer *l) {
	if (l->pos >= l->len) {
		l->width = 0;
		return eof;
	}

	// TODO: handle UTF-8.
	int c = l->input[l->pos];
	l->pos += l->width;
	return c;
}

static inline void ignore(struct lexer *l) {
	l->start = l->pos;
}

static inline void backup(struct lexer *l) {
	l->pos -= l->width;
}

static inline int peek(struct lexer *l) {
	char ret = next(l);
	backup(l);

	return ret;
}

static inline int cur(struct lexer *l) {
	backup(l);
	return next(l);
}

static inline int contains(const char *valid, const char c) {
	return strchr(valid, c) != NULL;
}

static inline int accept(struct lexer *l, char *valid) {
	if (l->pos < l->len && contains(valid, next(l))) {
		return 1;
	}
	backup(l);
	return 0;
}

static inline void accept_run(struct lexer *l, char *valid) {
	int c = next(l);

	while (c != eof && contains(valid, c)) {
		c = next(l);
	}

	backup(l);
}

static inline void accept_until(struct lexer *l, char end) {
	for (char cur = next(l); cur != end && cur != eof; cur = next(l)) {}
	backup(l);
}

static inline void emit(struct lexer *l, enum item_type type) {
	struct string s = slice_str(&l->input[l->start], l->pos - l->start);

	l->items = realloc(l->items, sizeof(struct item) * ++l->nitems);
	l->items[l->nitems-1] = new_item(s, type, l->pos);
	l->start = l->pos;
}

static inline struct string current_str(struct lexer *l) {
	return slice_str(&l->input[l->start], l->pos - l->start);
}

static inline void ignore_spaces(struct lexer *l) {
	accept_run(l, " \n\t\r\v\f");
	ignore(l);
}

static inline int is_space(int c) {
	return c == ' ' || c == '\n' || c == '\t';
}

static inline int is_number(int c) {
	return c == '+' || c == '-' || isdigit(c);
}

static void lex_expression(struct lexer *l);

static void lex_identifier(struct lexer *l) {
	char *accepted = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
	accept_run(l, accepted);
	emit(l, lookup_type(current_str(l)));
	l->state = lex_expression;
}

static void lex_plus(struct lexer *l) {
	switch (next(l)) {
	case '=':
		emit(l, item_plus_assign);
		break;

	case '+':
		emit(l, item_plusplus);
		break;

	default:
		backup(l);
		emit(l, item_plus);
		break;
	}

	l->state = lex_expression;
}

static void lex_minus(struct lexer *l) {
	switch (next(l)) {
	case '=':
		emit(l, item_minus_assign);
		break;

	case '+':
		emit(l, item_minusminus);
		break;

	default:
		backup(l);
		emit(l, item_minus);
		break;
	}

	l->state = lex_expression;
}

static void lex_times(struct lexer *l) {
	switch (next(l)) {
	case '=':
		emit(l, item_asterisk_assign);
		break;

	default:
		backup(l);
		emit(l, item_asterisk);
		break;
	}

	l->state = lex_expression;
}

static void lex_slash(struct lexer *l) {
	switch (next(l)) {
	case '=':
		emit(l, item_slash_assign);
		break;

	default:
		backup(l);
		emit(l, item_asterisk);
		break;
	}

	l->state = lex_expression;
}

static void lex_mod(struct lexer *l) {
	switch (next(l)) {
	case '=':
		emit(l, item_modulus_assign);
		break;

	default:
		backup(l);
		emit(l, item_modulus);
		break;
	}

	l->state = lex_expression;
}

static void lex_string(struct lexer *l) {
	for (;;) {
		switch (next(l)) {
		case '\\': {
			int n = next(l);
			if (n != eof && n != '\n') {
				break;
			}
		}

		case eof:
		case '\n':
			puts("lexer: unterminated quoted string");
			l->state = NULL;
			return;

		case '"':
			backup(l);
			goto end;
		}
	}

end:
	emit(l, item_string);
	next(l);
	ignore(l);
	l->state = lex_expression;
}

static void lex_raw_string(struct lexer *l) {
	if (peek(l) == '`') {
		emit(l, item_rawstring);
		next(l);
		ignore(l);
		l->state = lex_expression;
	}
	next(l);
	l->state = lex_raw_string;
}

static void lex_number(struct lexer *l) {
	enum item_type type = item_int;
	char *digits = "0123456789";

	// Optional leading sign.
	accept(l, "+-");

	// Is it hex?
	if (accept(l, "0") && accept(l, "xX")) {
		digits = "0123456789abcdefABCDEF";
	}

	accept_run(l, digits);

	// Is it a float?
	if (accept(l, ".")) {
		type = item_float;
		accept_run(l, digits);
	}

	if (accept(l, "eE")) {
		type = item_float;
		accept(l, "+-");
		accept(l, "0123456789");
	}

	emit(l, type);
	l->state = lex_expression;
}

static void lex_less_than(struct lexer *l) {
	switch (next(l)) {
	case '=':
		emit(l, item_lteq);
		break;

	case '<': {
		if (next(l) == '=') {
			emit(l, item_lshift_assign);
			break;
		}
		backup(l);
		emit(l, item_lshift);
		break;
	}

	default:
		backup(l);
		emit(l, item_lt);
		break;
	}

	l->state = lex_expression;
}

static void lex_greater_than(struct lexer *l) {
	switch (next(l)) {
	case '=':
		emit(l, item_gteq);
		break;

	case '>': {
		if (next(l) == '=') {
			emit(l, item_rshift_assign);
			break;
		}
		backup(l);
		emit(l, item_rshift);
		break;
	}

	default:
		backup(l);
		emit(l, item_gt);
		break;
	}

	l->state = lex_expression;
}

static void lex_and(struct lexer *l) {
	switch (next(l)) {
	case '&':
		emit(l, item_and);
		break;

	case '=':
		emit(l, item_bw_and_assign);
		break;

	default:
		backup(l);
		emit(l, item_bw_and);
		break;
	}

	l->state = lex_expression;
}

static void lex_or(struct lexer *l) {
	switch (next(l)) {
	case '|':
		emit(l, item_or);
		break;

	case '=':
		emit(l, item_bw_or_assign);
		break;

	default:
		backup(l);
		emit(l, item_bw_or);
		break;
	}

	l->state = lex_expression;
}

static void lex_xor(struct lexer *l) {
	switch (next(l)) {
	case '=':
		emit(l, item_bw_xor_assign);
		break;

	default:
		backup(l);
		emit(l, item_bw_xor);
		break;
	}

	l->state = lex_expression;
}

static void lex_expression(struct lexer *l) {
	int c = next(l);

	if (is_space(c)) {
		ignore(l);
		return;
	} else if (isalpha(c)) {
		backup(l);
		SETSTATE(lex_identifier);
	}

	switch (c) {
	case '\n':
	case ';':
		emit(l, item_semicolon);
		ignore_spaces(l);
		return;

	case '(':
		emit(l, item_lparen);
		ignore_spaces(l);
		return;

	case ')':
		emit(l, item_rparen);
		ignore_spaces(l);
		return;

	case '{':
		emit(l, item_lbrace);
		ignore_spaces(l);
		return;

	case '}':
		emit(l, item_rbrace);
		ignore_spaces(l);
		return;

	case ',':
		emit(l, item_comma);
		ignore_spaces(l);
		return;

	case '+':
		SETSTATE(lex_plus);

	case '-':
		SETSTATE(lex_minus);

	case '*':
		SETSTATE(lex_times);

	case '/':
		SETSTATE(lex_slash);

	case '%':
		SETSTATE(lex_mod);

	case '=': {
		if (next(l) == '=') {
			emit(l, item_equals);
		} else {
			backup(l);
			emit(l, item_assign);
		}
		return;
	}

	case '!':
		if (next(l) == '=') {
			emit(l, item_not_equals);
		} else {
			backup(l);
			emit(l, item_bang);
		}
		return;

	case '~':
		emit(l, item_bw_not);
		return;

	case '<':
		SETSTATE(lex_less_than);

	case '>':
		SETSTATE(lex_greater_than);

	case '&':
		SETSTATE(lex_and);

	case '|':
		SETSTATE(lex_or);

	case '^':
		SETSTATE(lex_xor);

	case '"':
		ignore(l);
		SETSTATE(lex_string);

	case '`':
		ignore(l);
		SETSTATE(lex_raw_string);

	case '[':
		emit(l, item_lbracket);
		ignore_spaces(l);
		return;

	case ']':
		emit(l, item_rbracket);
		return;

	case ':':
		emit(l, item_colon);
		return;

	case '.':
		emit(l, item_dot);
		ignore_spaces(l);
		return;

	case '#':
		accept_until(l, '\n');
		ignore_spaces(l);
		return;

	case eof:
		emit(l, item_eof);
		SETSTATE(NULL);

	default: {
		if (is_number(c)) {
			backup(l);
			SETSTATE(lex_number);
		}
		printf("lexer: invalid item \"%c\"\n", c);
		exit(1);
	}
	}

	l->state = lex_expression;
}

void lexer_run(struct lexer *l) {
	ignore_spaces(l);
	l->state = lex_expression;

	while (l->state != NULL) {
		l->state(l);
	}
}

struct lexer new_lexer(char *input, size_t len) {
	struct lexer l;

	l.input = input;
	l.len = len;
	l.start = 0;
	l.pos = 0;
	l.width = 1;
	l.nitems = 0;
	l.items = NULL;
	l.state = NULL;
	return l;
}

