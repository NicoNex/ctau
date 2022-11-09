#include <string.h>
#include <errno.h>
#include "parser.h"

enum precedence {
	lowest,
	assignment,
	logical_or,
	logical_and,
	bitwise_or,
	bitwise_and,
	equality,
	relational,
	shift,
	additive,
	multiplicative,
	prefix,
	call,
	index,
	dot
};

static inline enum precedence get_precedence(enum item_type type);
static inline prefixfn prefix_parser(enum item_type type);
static inline infixfn infix_parser(enum item_type type);

static inline void enter_loop(struct parser *p) {
	p->nested_loops++;
}

static inline void exit_loop(struct parser *p) {
	p->nested_loops--;
}

static inline int is_inside_loop(struct parser *p) {
	return p->nested_loops > 0;
}

static inline void next(struct parser *p) {
	p->cur = p->peek;
	p->peek = p->index + 1 < p->nitems ? p->items[++p->index] : NULL;
}

static inline int item_is(struct item i, enum item_type t) {
	return i.type == t;
}

static inline int cur_prec(struct parser *p) {
	return get_precedence(p->cur.type);
}

static inline enum precedence peek_prec(struct parser *p) {
	return get_precedence(p->peek.type);
}

static struct node *parse_expr(struct parser *p, enum precedence prec) {
	prefixfn pfn = prefix_parser(p->cur.type);

	if (pfn == NULL) {
		puts("no parser prefix function found");
		return NULL;
	}

	struct node *left = pfn(p);
	while (!item_is(p->peek, item_semicolon) && prec < peek_prec(p)) {
		infixfn ifn = infix_parser(p->peek.type);

		if (ifn == NULL) {
			break;
		}
		next(p);
		left = ifn(left);
	}

	if (item_is(p->peek, item_semicolon)) {
		next(p);
	}
	return left;
}

static node *parse_minus(struct parser *p, struct node *left) {
	enum precedence prec = cur_prec(p);
	next(p);
	return new_minus(left, parse_expr(p, prec));
}

static node *parse_plus(struct parser *p, struct node *left) {
	enum precedence prec = cur_prec(p);
	next(p);
	return new_plus(left, parse_expr(p, prec));
}

static node *parse_less(struct parser *p, struct node *left) {
	enum precedence prec = cur_prec(p);
	next(p);
	return new_less(left, parse_expr(p, prec));
}

static node *parse_identifier(struct parser *p) {
	char *name = calloc(p->cur.lit.len + 1, sizeof(char));
	strncpy(name, p->cur.lit.val, p->cur.lit.len);

	return new_identifier(name);
}

static node *parse_integer(struct parser *p) {
	char repr[p->cur.lit.len+1] = {0};
	strncpy(repr, p->cur.lit.val, p->cur.lit.len);

	int64_t *val = malloc(sizeof(int64_t));
	errno = 0;
	*val = strtol(repr, NULL, 10);

	if (errno != 0 && *val == 0) {
		printf("unable to parse \"%s\" as integer", repr);
		exit(1);
	}

	return new_integer(val);
}

static struct node *parse_return(struct parser *p) {
	struct node *ret;

	next(p);
	if (!item_is(p->cur, item_semicolon)) {
		ret = new_return(parse_expr(p, lowest));
	} else {
		ret = new_return(new_null());
	}

	if (item_is(p->peek, item_semicolon)) {
		next(p);
	}
	return ret;
}

static struct node *parse_statement(struct parser *p) {
	if (item_is(p->cur, item_return)) {
		return parse_return(p);
	}
	return parse_expr(p, lowest);
}

static struct node *parse(struct parser *p) {
	struct node *block = new_block();

	while (p->cur != NULL && !item_is(p->cur, item_eof)) {
		struct node *statement = parse_statement(p);
		if (statement != NULL) {
			block_add_statement(block->data, statement);
		}
		next(p);
	}

	return block;
}

struct parser new_parser(struct item *items, size_t nitems) {
	struct parser p;

	p.items = items;
	p.nitems = nitems;
	p.cur = nitems > 0 ? items[0] : NULL;
	p.peek = nitems > 1 ? items[1] : NULL;
	p.index = 1;
	p.nested_loops = 0;
	return p;
}

static inline enum precedence get_precedence(enum item_type type) {
	switch (type) {
	case item_assign:
	case item_plus_assign:
	case item_minus_assign:
	case item_slash_assign:
	case item_asterisk_assign:
	case item_modulus_assign:
	case item_bw_and_assign:
	case item_bw_or_assign:
	case item_bw_xor_assign:
	case item_lshift_assign:
	case item_rshift_assign:
		return assignment;
	case item_or:
		return logical_or;
	case item_and:
		return logical_and;
	case item_equals:
	case item_not_equals:
		return equality;
	case item_lt:
	case item_gt:
	case item_lteq:
	case item_gteq:
		return relational;
	case item_plus:
	case item_minus:
		return additive;
	case item_modulus:
	case item_slash:
	case item_asterisk:
		return multiplicative;
	case item_plusplus:
	case item_minusminus:
		return prefix;
	case item_bw_and:
		return bitwise_and;
	case item_bw_or:
	case item_bw_xor:
		return bitwise_or;
	case item_lshift:
	case item_rshift:
		return shift;
	case item_lparen:
		return call;
	case item_lbracket:
		return index;
	case item_dot:
		return dot;
	}
}

static inline prefixfn prefix_parser(enum item_type type) {
	switch (type) {
	case item_ident:
		return parse_identifier;
	case item_int:
		return parse_integer;
	// case item_float:
	// 	return parse_float;
	// case item_string:
	// 	return parse_string;
	// case item_rawstring:
	// 	return parse_rawstring;
	case item_minus:
		return parse_minus;
	// case item_bang:
	// 	return parse_bang;
	// case item_true:
	// 	return parse_true;
	// case item_false:
	// 	return parse_false;
	case item_lparen:
		return parse_grouped_expr;
	case item_if:
		return parse_if;
	case item_function:
		return parse_function;
	// case item_lbracket:
	// 	return parse_list;
	// case item_plusplus:
	// 	return parse_plusplus;
	// case item_minusminus:
	// 	return parse_minusminus;
	// case item_for:
	// 	return parse_for;
	// case item_lbrace:
	// 	return parse_map;
	// case item_null:
	// 	return parse_null;
	// case item_bwnot:
	// 	return parse_bwnot;
	// case item_continue:
	// 	return parse_continue;
	// case item_break:
	// 	return parse_break;
	// case item_import:
	// 	return parse_import;
	// case item_tau:
	// 	return parse_tau_call;
	default:
		return NULL;
	}
}

static inline infixfn infix_parser(enum item_type type) {
	switch (type) {
	// case item_equals:
	// 	return parse_equals;
	// case item_not_equals:
	// 	return parse_not_equals;
	case item_lt:
		return parse_less;
	// case item_gt:
	// 	return parse_greater;
	// case item_lteq:
	// 	return parse_lesseq;
	// case item_gteq:
	// 	return parse_greatereq;
	// case item_and:
	// 	return parse_and;
	// case item_or:
	// 	return parse_or;
	case item_plus:
		return parse_plus;
	case item_minus:
		return parse_minus;
	// case item_slash:
	// 	return parse_slash;
	// case item_asterisk:
	// 	return parse_asterisk;
	// case item_modulus:
	// 	return parse_modulus;
	// case item_bw_and:
	// 	return parse_bw_and;
	// case item_bw_or:
	// 	return parse_bw_or;
	// case item_bw_xor:
	// 	return parse_bw_xor;
	// case item_lshift:
	// 	return parse_lshift;
	// case item_rshift:
	// 	return parse_rshift;
	case item_assign:
		return parse_assign;
	// case item_plus_assign:
	// 	return parse_plus_assign;
	// case item_minus_assign:
	// 	return parse_minus_assign;
	// case item_slash_assign:
	// 	return parse_slash_assign;
	// case item_asterisk_assign:
	// 	return parse_asterisk_assign;
	// case item_modulus_assign:
	// 	return parse_modulus_assign;
	// case item_bw_and_assign:
	// 	return parse_bw_and_assign;
	// case item_bw_or_assign:
	// 	return parse_bw_or_assign;
	// case bw_xor_assign:
	// 	return parse_bw_xor_assign;
	// case item_lshift_assign:
	// 	return parse_lshift_assign;
	// case item_rshift_assign:
	// 	return parse_rshift_assign;
	case item_lparen:
		return parse_call;
	// case item_lbracket:
	// 	return parse_index;
	// case item_dot:
	// 	return parse_dot;
	default:
		return NULL;
	}
}
