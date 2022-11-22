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


