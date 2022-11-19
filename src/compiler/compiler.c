#include <stdlib.h>
#include "compiler.h"

int compiler_add_inst(struct compiler *c, uint8_t *ins, size_t len) {
	struct scope *scope = &c->scopes[c->nscopes-1];
	int offset = scope->ninsts;
	scope->ninsts += len;
	scope->insts = realloc(scope->insts, sizeof(uint8_t) * scope->ninsts);

	for (int i = offset; i < scope->ninsts; i++) {
		scope->insts[i] = ins[i % offset];
	}

	return scope->ninsts;
}

int compiler_add_const(struct compiler *c, struct obj *o) {
	c->consts = realloc(c->consts, sizeof(struct obj *) * ++c->nconsts);
	return c->nconsts - 1;
}

void compiler_set_last_inst(struct compiler *c, enum opcode op, int pos) {
	struct emitted_inst prev = c->scopes[c->scope_index].last_inst;
	struct emitted_inst last = {.opcode: op, .position: pos};
	c->scopes[c->scope_index].prev_inst = prev;
	c->scopes[c->scope_index].last_inst = last;
}

struct compiler *new_compiler_with_state(struct symbol_table *st, struct obj *consts) {
	struct compiler *c = calloc(1, sizeof(struct compiler));
	c->st = st;
	c->consts = consts;
	c->scopes = malloc(sizeof(struct scope));
	c->scopes[0] = {0};

	return c;
}

struct compiler *new_compiler() {
	struct compiler *c = calloc(1, sizeof(struct compiler));
	c->st = new_symbol_table();
	c->scopes = malloc(sizeof(struct scope));
	c->scopes[0] = {0};

	//TODO: define builtins.
	return c;
}
