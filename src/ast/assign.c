#include "ast.h"

struct assign {
	struct node *l;
	struct node *r;
};

int compile_assign(struct node *n, struct compiler *c) {
	return -1;
}

void dispose_assign(struct node *n) {
	struct assign *a = n->data;
	if (a->l != NULL) a->l->dispose(a->l);
	if (a->r != NULL) a->r->dispose(a->r);

	free(a);
	free(n);
}

struct node *new_assign(struct node *l, struct node *r) {
	struct assign *a = malloc(sizeof(struct assign));
	a->l = l;
	a->r = r;

	return new_node(a, assign_node, compile_assign, dispose_assign);
}
