#include "ast.h"

struct greater_node {
	struct node *l;
	struct node *r;
};

int compile_greater(struct node *n, struct compiler *c) {
	struct greater_node *g = n->data;

	CHECK(g->l->compile(g->l, c));
	CHECK(g->r->compile(g->r, c));
	return compiler_emit(c, op_greater_than);
}

void dispose_greater_node(struct node *n) {
	struct greater_node *g = n->data;
	if (g->l != NULL) g->l->dispose(g->l);
	if (g->r != NULL) g->r->dispose(g->r);

	free(g);
	free(n);
}

struct node *new_greater(struct node *l, struct node *r) {
	struct greater_node *g = malloc(sizeof(struct greater_node));
	g->l = l;
	g->r = r;

	return new_node(g, greater_node_t, compile_greater, dispose_greater_node);
}
