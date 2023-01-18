#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "item.h"

static inline int matches(struct string s, const char *t) {
	size_t tlen = strlen(t);
	return s.len == tlen && strncmp(s.val, t, tlen) == 0;
}

enum item_type lookup_type(struct string s) {
	if (matches(s, "fn")) {
		return item_function;
	} else if (matches(s, "for")) {
		return item_for;
	} else if (matches(s, "continue")) {
		return item_continue;
	} else if (matches(s, "break")) {
		return item_break;
	} else if (matches(s, "if")) {
		return item_if;
	} else if (matches(s, "else")) {
		return item_else;
	} else if (matches(s, "true")) {
		return item_true;
	} else if (matches(s, "false")) {
		return item_false;
	} else if (matches(s, "return")) {
		return item_return;
	} else if (matches(s, "null")) {
		return item_null;
	} else if (matches(s, "import")) {
		return item_import;
	} else if (matches(s, "tau")) {
		return item_tau;
	} else {
		return item_ident;
	}
}

struct item new_item(struct string lit, enum item_type type, int pos) {
	struct item i;

	i.type = type;
	i.lit = lit;
	i.pos = pos;
	return i;
}

struct string slice_str(char *s, size_t len) {
	struct string str;

	str.val = s;
	str.len = len;
	return str;
}

void print_item(struct item i) {
	char lit[i.lit.len+1];

	lit[i.lit.len] = '\0';
	strncpy(lit, i.lit.val, i.lit.len);
	printf("item: %s, type %d\n", lit, i.type);
}

inline const char *type_string(enum item_type t) {
	static const char *strings[] = {
		"item_eof",
		"item_error",
		"item_null",

		"item_ident",
		"item_int",
		"item_float",
		"item_string",
		"item_rawstring",

		"item_assign",
		"item_plus",
		"item_minus",
		"item_slash",
		"item_asterisk",
		"item_modulus",
		"item_plus_assign",
		"item_minus_assign",
		"item_asterisk_assign",
		"item_slash_assign",
		"item_modulus_assign",
		"item_bw_and_assign",
		"item_bw_or_assign",
		"item_bw_xor_assign",
		"item_lshift_assign",
		"item_rshift_assign",
		"item_equals",
		"item_not_equals",
		"item_bang",
		"item_lt",
		"item_gt",
		"item_lteq",
		"item_gteq",
		"item_and",
		"item_or",
		"item_bw_and",
		"item_bw_not",
		"item_bw_or",
		"item_bw_xor",
		"item_lshift",
		"item_rshift",
		"item_plusplus",
		"item_minusminus",

		"item_dot",
		"item_comma",
		"item_colon",
		"item_semicolon",
		"item_new_line",

		"item_lparen",
		"item_rparen",

		"item_lbrace",
		"item_rbrace",

		"item_lbracket",
		"item_rbracket",

		"item_function",
		"item_for",
		"item_continue",
		"item_break",
		"item_if",
		"item_else",
		"item_true",
		"item_false",
		"item_return",
		"item_import",
		"item_tau"
	};

	return strings[t];
}
