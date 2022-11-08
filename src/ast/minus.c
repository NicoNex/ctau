#include "ast.h"

struct minus {
	struct node *l;
	struct node *r;
};

int compile_minus(struct node *n, struct compiler *c) {
	return -1;
}

void dispose_minus(struct node *n) {
	puts("freeing minus");
	struct minus *m = (struct minus *) n->data;
	if (m->l != NULL) m->l->dispose(m->l);
	if (m->r != NULL) m->r->dispose(m->r);

	free(m);
	free(n);
}

struct node *new_minus(struct node *l, struct node *r) {
	struct minus *m = calloc(1, sizeof(struct minus));
	m->l = l;
	m->r = r;

	return new_node(m, MinusNode, compile_minus, dispose_minus);
}
