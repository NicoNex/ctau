#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "compiler.h"

int compiler_add_inst(struct compiler *c, uint8_t *ins, size_t len) {
	struct scope *scope = &c->scopes[c->nscopes-1];
	int offset = scope->len;
	scope->len += len;
	scope->insts = realloc(scope->insts, sizeof(uint8_t) * scope->len);

	for (int i = offset; i < scope->len; i++) {
		scope->insts[i] = ins[i % offset];
	}

	return scope->len;
}

int compiler_add_const(struct compiler *c, struct object *o) {
	int pos = c->nconsts;
	*c->consts = realloc(*c->consts, sizeof(struct object *) * ++c->nconsts);
	(*c->consts)[pos] = o;
	return pos;
}

void compiler_set_last_inst(struct compiler *c, enum opcode op, int pos) {
	struct emitted_inst prev = c->scopes[c->scope_index].last_inst;
	struct emitted_inst last = {.opcode = op, .position = pos};
	c->scopes[c->scope_index].prev_inst = prev;
	c->scopes[c->scope_index].last_inst = last;
}

int compiler_emit(struct compiler *c, enum opcode op, ...) {
	struct scope *scope = &c->scopes[c->scope_index];
	uint8_t **code = &scope->insts;
	int pos = scope->len;

	va_list args;
	va_start(args, op);
	scope->len = vmake_bcode(code, pos, op, args);
	va_end(args);
	compiler_set_last_inst(c, op, pos);

	return pos;
}

int compiler_last_is(struct compiler *c, uint8_t op) {
	struct scope scope = c->scopes[c->scope_index];
	return scope.len != 0 && scope.last_inst.opcode == op;
}

void compiler_remove_last(struct compiler *c) {
	struct emitted_inst last = c->scopes[c->scope_index].last_inst;
	struct emitted_inst prev = c->scopes[c->scope_index].prev_inst;

	uint8_t **old = &c->scopes[c->scope_index].insts;
	c->scopes[c->scope_index].len = last.position;
	*old = realloc(*old, sizeof(uint8_t) * last.position);
	c->scopes[c->scope_index].last_inst = prev;
}

void compiler_replace_inst(struct compiler *c, int pos, uint8_t *new, size_t len) {
	uint8_t *insts = c->scopes[c->scope_index].insts;

	for (int i = 0; i < len; i++) {
		insts[pos+i] = new[i];
	}
}

void compiler_replace_operand(struct compiler *c, int op_pos, int operand) {
	enum opcode op = c->scopes[c->scope_index].insts[op_pos];
	uint8_t *new = NULL;
	size_t len = make_bcode(&new, 0, op, operand);
	compiler_replace_inst(c, op_pos, new, len);
	free(new);
}

int compiler_replace_continue_operand(struct compiler *c, int start, int end, int operand) {
	uint8_t *insts = c->scopes[c->scope_index].insts;
	size_t len = c->scopes[c->scope_index].len;

	if (start > len || end > len) {
		puts("compiler error: start or end position out of range");
		return 0;
	}

	for (int i = start; i < end && i < len;) {
		struct definition *def = NULL;
		int found = lookup_def(insts[i], def);

		if (!found) {
			puts("compiler error: definition not found");
			return 0;
		}
		int *operands = NULL;
		int offset = read_operands(*def, insts, &operands);
		enum opcode op = insts[i];

		if (op == op_jump && operands[0] == CONTINUE_PLACEHOLDER) {
			compiler_replace_operand(c, i, operand);
		}
		free(operands);

		i += offset + 1;
	}

	return 1;
}

int compiler_replace_break_operand(struct compiler *c, int start, int end, int operand) {
	uint8_t *insts = c->scopes[c->scope_index].insts;
	size_t len = c->scopes[c->scope_index].len;

	if (start > len || end > len) {
		puts("compiler error: start or end position out of range");
		return 0;
	}

	for (int i = start; i < end && i < len;) {
		struct definition *def = NULL;
		int found = lookup_def(insts[i], def);

		if (!found) {
			puts("compiler error: definition not found");
			return 0;
		}
		int *operands = NULL;
		int offset = read_operands(*def, insts, &operands);
		enum opcode op = insts[i];

		if (op == op_jump && operands[0] == BREAK_PLACEHOLDER) {
			compiler_replace_operand(c, i, operand);
		}
		free(operands);

		i += offset + 1;
	}

	return 1;
}

void compiler_replace_last_pop_with_return(struct compiler *c) {
	int pos = c->scopes[c->scope_index].last_inst.position;

	uint8_t *new = NULL;
	size_t len = make_bcode(&new, 0, op_return_value);
	compiler_replace_inst(c, pos, new, len);
	free(new);
	c->scopes[c->scope_index].last_inst.opcode = op_return_value;
}

void compiler_enter_scope(struct compiler *c) {
	c->scopes = realloc(c->scopes, sizeof(struct scope) * ++c->nscopes);
	c->scope_index++;
	c->scopes[c->nscopes-1] = (struct scope ) {0};
	c->st = new_enclosed_symbol_table(c->st);
}

uint8_t *compiler_leave_scope(struct compiler *c, size_t *len) {
	uint8_t *insts = c->scopes[c->scope_index].insts;
	*len = c->scopes[c->scope_index].len;
	c->scopes = realloc(c->scopes, sizeof(struct scope) * --c->nscopes);
	c->scope_index--;
	struct symbol_table *oldst = c->st;
	c->st = c->st->outer;
	symbol_table_free(oldst);

	return insts;
}

int compiler_pos(struct compiler *c) {
	return c->scopes[c->scope_index].len;
}

int compile(struct compiler *c, struct node *tree) {
	tree->compile(tree, c);
	return compiler_emit(c, op_halt);
}

struct symbol *compiler_define(struct compiler *c, char *name) {
	return symbol_table_define(c->st, name);
}

struct symbol *compiler_resolve(struct compiler *c, char *name) {
	return symbol_table_resolve(c->st, name);
}

int compiler_load_symbol(struct compiler *c, struct symbol *s) {
	switch (s->scope) {
	case global_scope:
		return compiler_emit(c, op_get_global, s->index);
	case local_scope:
		return compiler_emit(c, op_get_local, s->index);
	case builtin_scope:
		return compiler_emit(c, op_get_builtin, s->index);
	case free_scope:
		return compiler_emit(c, op_get_free, s->index);
	case function_scope:
		return compiler_emit(c, op_current_closure, s->index);
	default:
		return -1;
	}
}

struct bytecode compiler_bytecode(struct compiler *c) {
	return (struct bytecode) {
		.insts = c->scopes[c->scope_index].insts,
		.consts = *c->consts,
		.len = c->scopes[c->scope_index].len,
		.nconsts = c->nconsts
	};
}

struct compiler *new_compiler_with_state(struct symbol_table *st, struct object ***consts, size_t nconsts) {
	struct compiler *c = calloc(1, sizeof(struct compiler));
	c->st = st;
	c->consts = consts;
	c->nconsts = nconsts;
	c->scopes = malloc(sizeof(struct scope));
	c->scopes[0] = (struct scope) {0};
	c->nscopes = 1;

	return c;
}

struct compiler *new_compiler() {
	struct compiler *c = calloc(1, sizeof(struct compiler));
	c->st = new_symbol_table();
	c->scopes = malloc(sizeof(struct scope));
	c->scopes[0] = (struct scope) {0};
	c->nscopes = 1;
	// TODO: find an elegant way to free this address.
	c->consts = calloc(1, sizeof(struct object **));

	//TODO: define builtins.
	return c;
}

void compiler_dispose(struct compiler *c) {
	free(c->scopes);
	free(c);
}
