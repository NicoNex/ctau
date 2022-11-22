#include "ast.h"

int compile_identifier(struct node *n, struct compiler *c) {
	return -1;
}

void dispose_identifier(struct node *n) {
	free(n->data);
	free(n);
}

struct node *new_identifier(char *name) {
	return new_node(name, identifier_node_t, compile_identifier, dispose_identifier);
}

