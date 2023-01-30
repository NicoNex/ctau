#include "ast.h"

struct less_node {
	struct node *l;
	struct node *r;
};

int compile_less(struct node *n, struct compiler *c) {
	struct less_node *l = n->data;

	// The order of the compilation of the operands is inverted
	// since we reuse the op_greater_than opcode.
	CHECK(l->r->compile(l->r, c));
	CHECK(l->l->compile(l->l, c));

	return compiler_emit(c, op_greater_than);
}

void dispose_less_node(struct node *n) {
	struct less_node *l = n->data;
	if (l->l != NULL) l->l->dispose(l->l);
	if (l->r != NULL) l->r->dispose(l->r);

	free(l);
	free(n);
}

struct node *new_less(struct node *l, struct node *r) {
	struct less_node *ln = malloc(sizeof(struct less_node));
	ln->l = l;
	ln->r = r;

	return new_node(ln, less_node_t, compile_less, dispose_less_node);
}
