#include "ast.h"

struct less_node {
	struct node *l;
	struct node *r;
};

int compile_less(struct node *n, struct compiler *c) {
	struct less_node *ln = n->data;

	// The order of the compilation of the operands is inverted
	// since we reuse the op_greater_than opcode.
	CHECK(ln->r->compile(ln->r, c));
	CHECK(ln->l->compile(ln->l, c));

	return compiler_emit(c, op_greater_than);
}

void dispose_less(struct node *n) {
	struct less_node *l = n->data;
	if (l->l != NULL) l->l->dispose(l->l);
	if (l->r != NULL) l->r->dispose(l->r);

	free(l);
	free(n);
}

struct node *new_less(struct node *l, struct node *r) {
	struct less_node *less = malloc(sizeof(struct less_node));
	less->l = l;
	less->r = r;

	return new_node(less, less_node_t, compile_less, dispose_less);
}

