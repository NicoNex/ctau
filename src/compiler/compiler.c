#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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
	struct emitted_inst last = {.opcode = op, .position = pos};
	c->scopes[c->scope_index].prev_inst = prev;
	c->scopes[c->scope_index].last_inst = last;
}

int compiler_emit(struct compiler *c, enum opcode op, ...) {
	uint8_t **code = &c->scopes[c->scope_index].insts;
	size_t len = c->scopes[c->scope_index].ninsts;

	va_list args;
	va_start(args, op);
	int pos = make_bcode(code, len, op, args);
	compiler_set_last_inst(c, op, pos);

	return pos;
}

int compiler_last_is(struct compiler *c, uint8_t op) {
	struct scope scope = c->scopes[c->scope_index];
	return scope.ninsts != 0 && scope.last_inst.opcode == op;
}

void compiler_remove_last(struct compiler *c) {
	struct emitted_inst last = c->scopes[c->scope_index].last_inst;
	struct emitted_inst prev = c->scopes[c->scope_index].prev_inst;

	uint8_t **old = &c->scopes[c->scope_index].insts;
	c->scopes[c->scope_index].ninsts = last.position;
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
	size_t len = c->scopes[c->scope_index].ninsts;

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
	size_t len = c->scopes[c->scope_index].ninsts;

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

struct compiler *new_compiler_with_state(struct symbol_table *st, struct obj **consts) {
	struct compiler *c = calloc(1, sizeof(struct compiler));
	c->st = st;
	c->consts = consts;
	c->scopes = malloc(sizeof(struct scope));
	c->scopes[0] = (struct scope) {0};

	return c;
}

struct compiler *new_compiler() {
	struct compiler *c = calloc(1, sizeof(struct compiler));
	c->st = new_symbol_table();
	c->scopes = malloc(sizeof(struct scope));
	c->scopes[0] = (struct scope) {0};

	//TODO: define builtins.
	return c;
}
