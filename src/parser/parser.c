#include <string.h>
#include <errno.h>
#include "parser.h"
#include "../lexer/lexer.h"

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
	indexprec,
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

	if (p->index + 1 < p->nitems) {
		p->peek = p->items[++p->index];
	}
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

static inline int expect_peek(struct parser *p, enum item_type t) {
	if (item_is(p->peek, t)) {
		next(p);
		return 1;
	}
	printf("expected next item to be '%s' but got '%s' instead\n", itype_lit(t), itype_lit(p->peek.type));
	return 0;
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
		left = ifn(p, left);
	}

	if (item_is(p->peek, item_semicolon)) {
		next(p);
	}
	return left;
}

static struct node *parse_minus(struct parser *p, struct node *left) {
	enum precedence prec = cur_prec(p);
	next(p);
	return new_minus(left, parse_expr(p, prec));
}

static struct node *parse_plus(struct parser *p, struct node *left) {
	enum precedence prec = cur_prec(p);
	next(p);
	return new_plus(left, parse_expr(p, prec));
}

static struct node *parse_less(struct parser *p, struct node *left) {
	enum precedence prec = cur_prec(p);
	next(p);
	return new_less(left, parse_expr(p, prec));
}

static struct node *parse_assign(struct parser *p, struct node *left) {
	next(p);
	return new_assign(left, parse_expr(p, lowest));
}

static size_t parse_node_sequence(struct parser *p, struct node ***nodelist, enum item_type sep, enum item_type end) {
	size_t len = 0;
	next(p);

	if (item_is(p->cur, end)) {
		return len;
	}

	*nodelist = realloc(*nodelist, sizeof(struct node *) * ++len);
	*nodelist[len-1] = parse_expr(p, lowest);

	while (item_is(p->peek, sep)) {
		next(p);
		next(p);
		*nodelist = realloc(*nodelist, sizeof(struct node *) * ++len);
		*nodelist[len-1] = parse_expr(p, lowest);
	}

	if (!expect_peek(p, end)) {
		puts("node sequence not terminated");
		exit(1);
	}

	return len;
}

static struct node *parse_call(struct parser *p, struct node *fn) {
	struct node **nodelist = NULL;
	size_t len = parse_node_sequence(p, &nodelist, item_comma, item_rparen);

	return new_call(fn, nodelist, len);
}

static struct node *parse_identifier(struct parser *p) {
	char *name = calloc(p->cur.lit.len + 1, sizeof(char));
	strncpy(name, p->cur.lit.val, p->cur.lit.len);

	return new_identifier(name);
}

static struct node *parse_integer(struct parser *p) {
	char repr[p->cur.lit.len+1];
	repr[p->cur.lit.len] = '\0';
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

static struct node *parse_grouped_expr(struct parser *p) {
	next(p);
	struct node *expr = parse_expr(p, lowest);
	if (!expect_peek(p, item_rparen)) {
		exit(1);
	}

	return expr;
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

static struct node *parse_block(struct parser *p) {
	struct node *block = new_block();
	next(p);

	while (!item_is(p->cur, item_rbrace) && !item_is(p->cur, item_eof)) {
		struct node *statement = parse_statement(p);

		if (statement != NULL) {
			block_add_statement(block->data, statement);
		}
		next(p);
	}

	if (!item_is(p->cur, item_rbrace)) {
		// puts("expected \"}\" after block");
		exit(1);
	}

	return block;
}

static size_t parse_function_params(struct parser *p, char ***params) {
	size_t len = 0;

	if (item_is(p->peek, item_rparen)) {
		next(p);
		return len;
	}

	next(p);
	char *param = calloc(p->cur.lit.len+1, sizeof(char));
	strncpy(param, p->cur.lit.val, p->cur.lit.len);

	*params = realloc(*params, sizeof(char **));
	*params[0] = param;
	len += 1;

	while (item_is(p->peek, item_comma)) {
		next(p);
		next(p);

		param = calloc(p->cur.lit.len+1, sizeof(char));
		strncpy(param, p->cur.lit.val, p->cur.lit.len);

		*params = realloc(*params, sizeof(char **) * ++len);
		*params[len-1] = param;
	}

	if (!expect_peek(p, item_rparen)) {
		exit(1);
	}

	return len;
}

static struct node *parse_function(struct parser *p) {
	if (!expect_peek(p, item_lparen)) {
		exit(1);
	}

	char **params = NULL;
	size_t nparams = parse_function_params(p, &params);

	if (!expect_peek(p, item_lbrace)) {
		exit(1);
	}

	return new_function(params, nparams, parse_block(p));
}

static struct node *parse_ifexpr(struct parser *p) {
	next(p);
	struct node *cond = parse_expr(p, lowest);

	if (!expect_peek(p, item_lbrace)) {
		// puts("expecting \"{\" after if keyword");
		print_item(p->cur);
		print_item(p->peek);
		exit(1);
	}

	struct node *body = parse_block(p);
	struct node *alt = NULL;

	if (item_is(p->peek, item_else)) {
		next(p);

		if (item_is(p->peek, item_if)) {
			next(p);
			alt = parse_ifexpr(p);
		} else {
			if (!expect_peek(p, item_lbrace)) {
				// puts("expecting \"{\" after else keyword");
				exit(1);
			}
			alt = parse_block(p);
		}
	}

	return new_ifelse(cond, body, alt);
}

static struct node *parse(struct parser *p) {
	struct node *block = new_block();

	while (!item_is(p->cur, item_eof)) {
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

	if (nitems > 0) p.cur = items[0];
	if (nitems > 1) p.peek = items[1];
	p.items = items;
	p.nitems = nitems;
	p.index = 1;
	p.nested_loops = 0;
	return p;
}

struct node *parse_input(char *input, size_t len) {
	struct lexer l = new_lexer(input, len);
	lexer_run(&l);

	struct parser p = new_parser(l.items, l.nitems);
	struct node *tree = parse(&p);
	free(l.items);

	return tree;
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
		return indexprec;
	case item_dot:
		return dot;
	default:
		return lowest;
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
	// case item_minus:
	// 	return parse_prefix_minus;
	// case item_bang:
	// 	return parse_bang;
	// case item_true:
	// 	return parse_true;
	// case item_false:
	// 	return parse_false;
	case item_lparen:
		return parse_grouped_expr;
	case item_if:
		return parse_ifexpr;
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

