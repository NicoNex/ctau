#include "ast.h"

struct assign {
	struct node *l;
	struct node *r;
};

int compile_assign(struct node *n, struct compiler *c) {
	struct assign *a = n->data;
	int pos = 0;

	switch (a->l->type) {
	case identifier_node: {
		struct symbol *s = compiler_define(c, a->l->data);
		CHECK(a->r->compile(a->r, c));

		enum opcode op = s->scope == global_scope ? op_set_global : op_set_local;
		return compiler_emit(c, op, s->index);
	}

	// case dot_node:
	// case index_node:
	// 	puts("assign: node not yet supported");
	// 	return -1;

	default:
		puts("cannot assign to literal");
		return -1;
	}
}

void dispose_assign(struct node *n) {
	struct assign *a = n->data;
	if (a->l != NULL) a->l->dispose(a->l);
	if (a->r != NULL) a->r->dispose(a->r);

	free(a);
	free(n);
}

struct node *new_assign(struct node *l, struct node *r) {
	struct assign *a = malloc(sizeof(struct assign));
	a->l = l;
	a->r = r;

	return new_node(a, assign_node, compile_assign, dispose_assign);
}
