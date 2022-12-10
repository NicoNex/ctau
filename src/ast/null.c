#include "ast.h"
#include "../obj/obj.h"

int compile_null(struct node *n, struct compiler *c) {
	return compiler_emit(c, op_constant, compiler_add_const(c, null_obj));
}

void dispose_null_node(struct node *n) {
	free(n);
}

struct node *new_null() {
	return new_node(NULL, null_node_t, compile_null, dispose_null_node);
}

