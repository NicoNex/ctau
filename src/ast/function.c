#include "ast.h"

struct function {
	struct node *body;
	char **params;
	size_t nparams;
};

int compile_function(struct node *n, struct compiler *c) {
	return -1;
}

void dispose_function(struct node *n) {
	struct function *f = n->data;

	if (f->body != NULL) f->body->dispose(f->body);
	if (f->params != NULL) {
		for (int i = 0; i < f->nparams; i++) {
			free(f->params[i]);
		}
	}

	free(f);
	free(n);
}

struct node *new_function(char **params, size_t nparams, struct node *body) {
	struct function *f = malloc(sizeof(struct function));
	f->body = body;
	f->params = params;
	f->nparams = nparams;

	return new_node(f, function_node, compile_function, dispose_function);
}
