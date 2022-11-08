#include <ctype.h>
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

static inline int index_char(const char *valid, const char c) {
	return strchr(valid, c) - valid;
}

static inline int accept(struct lexer *l, char *valid) {
	if (index_char(valid, next(l)) >= 0) {
		return 1;
	}
	backup(l);
	return 0;
}

static inline void accept_run(struct lexer *l, char *valid) {
	while (index_char(valid, next(l)) >= 0) {}
	backup(l);
}

static inline void accept_until(struct lexer *l, char end) {
	for (char cur = next(l); cur != end && cur != eof; cur = next(l)) {}
	backup(l);
}

static inline void emit(struct lexer *l, enum item_type type) {
	struct string s = slice_str(l->input + l->start, l->pos - l->start);

	l->items = reallocarray(l->items, ++l->nitems, sizeof(struct item *));
	l->items[l->nitems-1] = new_item(s, type, l->pos);
}

static inline struct string current_str(struct lexer *l) {
	return slice_str(l->input, l->start, l->pos);
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

static void lex_expression(const struct lexer *l);

static void lex_identifier(const struct lexer *l) {
	char *accepted = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
	accept_run(l, accepted);
	emit(l, lookup_type(current_str(l)));
	l->state = lex_expression;
}

static void lex_plus(const struct lexer *l) {
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

static void lex_minus(const struct lexer *l) {
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

static void lex_number(const struct lexer *l) {
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
		accept_run(l, digits)
	}

	if (accept(l, "eE")) {
		type = item_float;
		accept(l, "+-");
		accept(l, "0123456789");
	}

	emit(l, type);
	l->state = lex_expression;
}

static void lex_expression(const struct lexer *l) {
	int c = next(l);

	if (is_space(c)) {
		ignore(l);
		goto end;
	} else if (isalpha(c)) {
		backup(l);
		SETSTATE(lex_identifier);
	}

	switch (c) {
	case '\n':
		emit(l, item_semicolon);
		ignore_spaces(l);
		break;

	case '(':
		emit(l, item_lparen);
		ignore_spaces(l);
		break;

	case ')':
		emit(l, item_rparen);
		ignore_spaces(l);
		break;

	case '{':
		emit(l, item_lbrace);
		ignore_spaces(l);
		break;

	case '}':
		emit(l, item_rbrace);
		ignore_spaces(l);
		break;

	case ',':
		emit(l, item_comma);
		ignore_spaces(l);
		break;

	case '+':
		SETSTATE(lex_plus);

	case '-':
		SETSTATE(lex_minus);

	default: {
		if (is_number(c)) {
			backup(l);
			SETSTATE(lex_number);
		}
		printf("lexer: invalid item \"%c\"", c);
		exit(1);
	}
	}

end:
	l->state = lex_expression;
}

static void run(const struct lexer *l) {
	ignore_spaces(l);
	l->state = lex_expression;

	while (l->state != NULL) {
		l->state(l);
	}
}
