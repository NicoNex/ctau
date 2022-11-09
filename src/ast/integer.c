#include <stdint.h>
#include "ast.h"

int compile_integer(struct node *n, struct compiler *c) {
	return -1;
}

void dispose_integer(struct node *n) {
	free(n->data);
	free(n);
}

struct node *new_integer(int64_t *val) {
	return new_node(val, integer_node, compile_integer, dispose_integer);
}
