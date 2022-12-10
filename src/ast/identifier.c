#include <stdio.h>
#include "ast.h"

int compile_identifier(struct node *n, struct compiler *c) {
	struct symbol *s = compiler_resolve(c, n->data);

	if (s == NULL) {
		printf("undefined variable %s\n", (char *) n->data);
		return -1;
	}
	return compiler_load_symbol(c, s);
}

void dispose_identifier_node(struct node *n) {
	free(n->data);
	free(n);
}

struct node *new_identifier(char *name) {
	return new_node(name, identifier_node_t, compile_identifier, dispose_identifier_node);
}

