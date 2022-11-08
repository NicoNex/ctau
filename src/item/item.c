#include <string.h>
#include "item.h"

#define MIN(a, b) a < b ? a : b

enum item_type lookup_type(struct string s) {
	if (strncmp(s.ident, "fn", MIN(s.len, 2)) == 0) {
		return item_function;
	} else if (strncmp(s.ident, "for", MIN(s.len, 3)) == 0) {
		return item_for;
	} else if (strncmp(s.ident, "continue", MIN(s.len, 8)) == 0) {
		return item_continue;
	} else if (strncmp(s.ident, "break", MIN(s.len, 5)) == 0) {
		return item_break;
	} else if (strncmp(s.ident, "if", MIN(s.len, 2)) == 0) {
		return item_if;
	} else if (strncmp(s.ident, "else", MIN(s.len, 4)) == 0) {
		return item_else;
	} else if (strncmp(s.ident, "true", MIN(s.len, 4)) == 0) {
		return item_true;
	} else if (strncmp(s.ident, "false", MIN(s.len, 5)) == 0) {
		return item_false;
	} else if (strncmp(s.ident, "return") == 0) {
		return item_return;
	} else if (strncmp(s.ident, "null", MIN(s.len, 4)) == 0) {
		return item_null;
	} else if (strncmp(s.ident, "import", MIN(s.len, 6)) == 0) {
		return item_import;
	} else if (strncmp(s.ident, "tau", MIN(s.len, 3)) == 0) {
		return item_tau;
	} else {
		return item_ident;
	}
}

struct item *new_item(struct string lit, enum item_type type, int pos) {
	struct item *i = malloc(sizeof(struct item));

	i->type = type;
	i->lit = lit;
	i->pos = pos;
	return i;
}

struct string slice_str(const char *s, size_t len) {
	return {.val = s, .len = len};
}
