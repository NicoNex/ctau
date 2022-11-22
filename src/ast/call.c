#include "ast.h"

struct call_node {
	struct node *fn;
	struct node **args;
	size_t arglen;
};

int compile_call(struct node *n, struct compiler *c) {
	struct call_node *call = n->data;
	int pos = 0;

	CHECK(pos = call->fn->compile(call->fn, c));

	for (int i = 0; i < call->arglen; i++) {
		struct node *arg = call->args[i];
		CHECK(pos = arg->compile(arg, c));
	}

	return compiler_emit(c, op_call, call->arglen);
}

void dispose_call(struct node *n) {
	struct call_node *c = n->data;

	if (c->fn != NULL) c->fn->dispose(c->fn);

	if (c->args != NULL) {
		for (int i = 0; i < c->arglen; i++) {
			c->args[i]->dispose(c->args[i]);
		}
	}

	free(c);
	free(n);
}

struct node *new_call(struct node *fn, struct node **args, size_t arglen) {
	struct call_node *c = malloc(sizeof(struct call_node));
	c->fn = fn;
	c->args = args;
	c->arglen = arglen;

	return new_node(c, call_node_t, compile_call, dispose_call);
}

