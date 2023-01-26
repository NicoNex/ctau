#include "ast.h"
#include "../obj/obj.h"

struct function_node {
	struct node *body;
	char **params;
	size_t nparams;
};

int compile_function(struct node *n, struct compiler *c) {
	struct function_node *fn = n->data;

	compiler_enter_scope(c);
	for (int i = 0; i < fn->nparams; i++) {
		compiler_define(c, fn->params[i]);
	}

	CHECK(fn->body->compile(fn->body, c));

	if (compiler_last_is(c, op_pop)) {
		compiler_replace_last_pop_with_return(c);
	}
	if (!compiler_last_is(c, op_return_value)) {
		compiler_emit(c, op_return);
	}

	struct symbol **free_symbols = c->st->free_symbols;
	size_t nfree = c->st->nfree;
	int num_locals = c->st->num_defs;
	size_t inslen = 0;
	uint8_t *insts = compiler_leave_scope(c, &inslen);

	for (int i = 0; i < nfree; i++) {
		CHECK(compiler_load_symbol(c, free_symbols[i]));
	}

	struct object *fnobj = new_function_obj(insts, inslen, num_locals, fn->nparams);
	int fnpos = compiler_add_const(c, fnobj);
	return compiler_emit(c, op_closure, fnpos, nfree);
}

void dispose_function_node(struct node *n) {
	struct function_node *f = n->data;

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
	struct function_node *f = malloc(sizeof(struct function));
	f->body = body;
	f->params = params;
	f->nparams = nparams;

	return new_node(f, function_node_t, compile_function, dispose_function_node);
}

