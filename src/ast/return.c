#include "ast.h"

struct return_node {
	struct node *val;
};

int compile_return(struct node *n, struct compiler *c) {
	struct return_node *ret = n->data;

	CHECK(ret->val->compile(ret->val, c));
	return compiler_emit(c, op_return_value);
}

void dispose_return(struct node *n) {
	struct return_node *r = n->data;
	if (r->val != NULL) r->val->dispose(r->val);

	free(r);
	free(n);
}

struct node *new_return(struct node *val) {
	struct return_node *r = malloc(sizeof(struct return_node));
	r->val = val;

	return new_node(r, return_node_t, compile_return, dispose_return);
}

