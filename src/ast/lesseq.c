#include "ast.h"

struct less_eq_node {
	struct node *l;
	struct node *r;
};

int compile_less_eq(struct node *n, struct compiler *c) {
	struct less_eq_node *ln = n->data;

	// The order of the compilation of the operands is inverted
	// since we reuse the op_greater_than opcode.
	CHECK(ln->r->compile(ln->r, c));
	CHECK(ln->l->compile(ln->l, c));

	return compiler_emit(c, op_greater_than_equal);
}

void dispose_less_eq_node(struct node *n) {
	struct less_eq_node *l = n->data;
	if (l->l != NULL) l->l->dispose(l->l);
	if (l->r != NULL) l->r->dispose(l->r);

	free(l);
	free(n);
}

struct node *new_less_eq(struct node *l, struct node *r) {
	struct less_eq_node *ln = malloc(sizeof(struct less_eq_node));
	ln->l = l;
	ln->r = r;

	return new_node(ln, lesseq_node_t, compile_less_eq, dispose_less_eq_node);
}
