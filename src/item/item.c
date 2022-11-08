#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "item.h"

#define MIN(a, b) a < b ? a : b

enum item_type lookup_type(struct string s) {
	if (strncmp(s.val, "fn", MIN(s.len, 2)) == 0) {
		return item_function;
	} else if (strncmp(s.val, "for", MIN(s.len, 3)) == 0) {
		return item_for;
	} else if (strncmp(s.val, "continue", MIN(s.len, 8)) == 0) {
		return item_continue;
	} else if (strncmp(s.val, "break", MIN(s.len, 5)) == 0) {
		return item_break;
	} else if (strncmp(s.val, "if", MIN(s.len, 2)) == 0) {
		return item_if;
	} else if (strncmp(s.val, "else", MIN(s.len, 4)) == 0) {
		return item_else;
	} else if (strncmp(s.val, "true", MIN(s.len, 4)) == 0) {
		return item_true;
	} else if (strncmp(s.val, "false", MIN(s.len, 5)) == 0) {
		return item_false;
	} else if (strncmp(s.val, "return", MIN(s.len, 6)) == 0) {
		return item_return;
	} else if (strncmp(s.val, "null", MIN(s.len, 4)) == 0) {
		return item_null;
	} else if (strncmp(s.val, "import", MIN(s.len, 6)) == 0) {
		return item_import;
	} else if (strncmp(s.val, "tau", MIN(s.len, 3)) == 0) {
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

void print_item(struct item *i) {
	for (int j = 0; j < i->lit.len; j++) {
		putchar(i->lit.val[j]);
	}
	putchar('\n');
}
