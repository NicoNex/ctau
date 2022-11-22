#include "ast.h"

struct minus_node {
	struct node *l;
	struct node *r;
};

int compile_minus(struct node *n, struct compiler *c) {
	return -1;
}

void dispose_minus(struct node *n) {
	struct minus_node *m = n->data;
	if (m->l != NULL) m->l->dispose(m->l);
	if (m->r != NULL) m->r->dispose(m->r);

	free(m);
	free(n);
}

struct node *new_minus(struct node *l, struct node *r) {
	struct minus_node *m = malloc(sizeof(struct minus_node));
	m->l = l;
	m->r = r;

	return new_node(m, minus_node_t, compile_minus, dispose_minus);
}
