#include "ast.h"

struct less_node {
	struct node *l;
	struct node *r;
};

int compile_less(struct node *n, struct compiler *c) {
	return -1;
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

