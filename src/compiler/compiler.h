#ifndef COMPILER_H_
#define COMPILER_H_

#include <stddef.h>
#include <stdint.h>

#include "../ast/ast.h"
#include "../obj/obj.h"
#include "../code/code.h"
#include "../data/map.h"

#define CONTINUE_PLACEHOLDER 9998
#define BREAK_PLACEHOLDER 9997

enum symbol_scope {
	global_scope,
	local_scope,
	builtin_scope,
	free_scope,
	function_scope
};

struct symbol {
	char *name;
	enum symbol_scope scope;
	int index;
};

struct symbol_table {
	struct symbol_table *outer;
	strmap store;
	struct symbol **free_symbols;
	size_t nfree;
	int num_defs;
};

struct emitted_inst {
	enum opcode opcode;
	int position;
};

struct scope {
	uint8_t *insts;
	size_t ninsts;
	struct emitted_inst last_inst;
	struct emitted_inst prev_inst;
};

struct compiler {
	struct object **consts;
	size_t nconsts;
	struct scope *scopes;
	size_t nscopes;
	int scope_index;
	struct symbol_table *st;
};

struct bytecode {
	uint8_t *insts;
	struct object **consts;
	size_t ninsts;
	size_t nconsts;
};

int compiler_add_inst(struct compiler *c, uint8_t *ins, size_t len);
int compiler_add_const(struct compiler *c, struct object *o);
void compiler_set_last_inst(struct compiler *c, enum opcode op, int pos);
int compiler_emit(struct compiler *c, enum opcode op, ...);
int compiler_last_is(struct compiler *c, uint8_t op);
void compiler_remove_last(struct compiler *c);
void compiler_replace_inst(struct compiler *c, int pos, uint8_t *new, size_t len);
void compiler_replace_operand(struct compiler *c, int op_pos, int operand);
int compiler_replace_continue_operand(struct compiler *c, int start, int end, int operand);
int compiler_replace_break_operand(struct compiler *c, int start, int end, int operand);
void compiler_replace_last_pop_with_return(struct compiler *c);
void compiler_enter_scope(struct compiler *c);
uint8_t *compiler_leave_scope(struct compiler *c, size_t *len);
int compiler_pos(struct compiler *c);
struct symbol *compiler_define(struct compiler *c, char *name);
int compiler_load_symbol(struct compiler *c, struct symbol *s);
struct symbol *compiler_resolve(struct compiler *c, char *name);
struct bytecode compiler_bytecode(struct compiler *c);
struct compiler *new_compiler_with_state(struct symbol_table *st, struct object **consts);
struct compiler *new_compiler();

// TODO: eventually remove these.
struct symbol_table *new_symbol_table();
struct symbol_table *new_enclosed_symbol_table(struct symbol_table *outer);
void symbol_table_free(struct symbol_table *s);
struct symbol *symbol_table_define(struct symbol_table *s, char *name);
struct symbol *symbol_table_resolve(struct symbol_table *s, char *name);

#endif

