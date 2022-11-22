#include <stdint.h>
#include "ast.h"
#include "../obj/obj.h"

int compile_integer(struct node *n, struct compiler *c) {
	int pos = compiler_add_const(c, new_integer_obj(*(int64_t *)n->data));
	return compiler_emit(c, op_constant, pos);
}

void dispose_integer(struct node *n) {
	free(n->data);
	free(n);
}

struct node *new_integer(int64_t *val) {
	return new_node(val, integer_node_t, compile_integer, dispose_integer);
}

