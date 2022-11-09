#include "ast.h"

struct retnode {
	struct node *val;
};

int compile_return(struct node *n, struct compiler *c) {
	return -1;
}

void dispose_return(struct node *n) {
	struct retnode *r = n->data;
	if (r->val != NULL) r->val->dispose(r->val);

	free(r);
	free(n);
}

struct node *new_return(struct node *val) {
	struct retnode *r = malloc(sizeof(struct retnode));
	r->val = val;

	return new_node(r, return_node, compile_return, dispose_return);
}
