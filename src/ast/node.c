#include "ast.h"

struct node *new_node(void *data, enum type t, compilefn cfn, disposefn dfn) {
	struct node *n = calloc(1, sizeof(struct node));
	n->data = data;
	n->type = t;
	n->compile = cfn;
	n->dispose = dfn;

	return n;
}
