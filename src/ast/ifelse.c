#include "ast.h"

struct ifelse_node {
	struct node *cond;
	struct node *body;
	struct node *altern;
};

int compile_ifelse(struct node *n, struct compiler *c) {
	struct ifelse_node *ie = n->data;

	CHECK(ie->cond->compile(ie->cond, c));
	int jump_not_truthy_pos = compiler_emit(c, op_jump_not_truthy, 9999);
	CHECK(ie->body->compile(ie->body, c));

	if (compiler_last_is(c, op_pop)) {
		compiler_remove_last(c);
	}

	int jump_pos = compiler_emit(c, op_jump, 9999);
	compiler_replace_operand(c, jump_not_truthy_pos, compiler_pos(c));

	if (ie->altern != NULL) {
		CHECK(ie->altern->compile(ie->altern, c));

		if (compiler_last_is(c, op_pop)) {
			compiler_remove_last(c);
		}
	} else {
		compiler_emit(c, op_null);
	}

	compiler_replace_operand(c, jump_pos, compiler_pos(c));
	return compiler_pos(c);
}

void dispose_ifelse(struct node *n) {
	struct ifelse_node *i = n->data;
	if (i->cond != NULL) i->cond->dispose(i->cond);
	if (i->body != NULL) i->body->dispose(i->body);
	if (i->altern != NULL) i->altern->dispose(i->altern);

	free(i);
	free(n);
}

struct node *new_ifelse(struct node *cond, struct node *body, struct node *altern) {
	struct ifelse_node *i = malloc(sizeof(struct ifelse_node));
	i->cond = cond;
	i->body = body;
	i->altern = altern;

	return new_node(i, ifelse_node_t, compile_ifelse, dispose_ifelse);
}

