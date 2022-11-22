#include "ast.h"

struct plus_node {
	struct node *l;
	struct node *r;
};

int compile_plus(struct node *n, struct compiler *c) {
	struct plus_node *p = n->data;

	CHECK(p->l->compile(p->l, c));
	CHECK(p->r->compile(p->r, c));

	return compiler_emit(c, op_add);
}

void dispose_plus(struct node *n) {
	struct plus_node *p = n->data;
	if (p->l != NULL) p->l->dispose(p->l);
	if (p->r != NULL) p->r->dispose(p->r);

	free(p);
	free(n);
}

struct node *new_plus(struct node *l, struct node *r) {
	struct plus_node *p = malloc(sizeof(struct plus_node));
	p->l = l;
	p->r = r;

	return new_node(p, plus_node_t, compile_plus, dispose_plus);
}

