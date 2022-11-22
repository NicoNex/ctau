#include "ast.h"

int compile_null(struct node *n, struct compiler *c) {
	return -1;
}

void dispose_null(struct node *n) {
	free(n);
}

struct node *new_null() {
	return new_node(NULL, null_node_t, compile_null, dispose_null);
}
