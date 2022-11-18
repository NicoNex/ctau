#ifndef COMPILER_H_
#define COMPILER_H_

#include <stdint.h>

#include "../ast/ast.h"
#include "../obj/obj.h"
#include "../code/code.h"
#include "../data/map.h"

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

struct compilation_scope {
	uint8_t *instructions;
	size_t ninstructions;
	struct emitted_inst last_inst;
	struct emitted_inst prev_inst;
};

struct compiler {
	struct obj *constants;
	size_t nconstants;
	struct compilation_scope *scopes;
	size_t nscopes;
	int scope_index;
	struct symbol_table st;
};

struct bytecode {
	uint8_t *instructions;
	struct obj *constants;
	size_t ninstructions;
	size_t nconstants;
};

struct symbol_table *new_symbol_table();
struct symbol_table *new_enclosed_symbol_table(struct symbol_table *outer);
struct symbol *symbol_table_define(struct symbol_table *s, char *name);
struct symbol *symbol_table_define_free(struct symbol_table *s, struct symbol *original);
struct symbol *symbol_table_resolve(struct symbol_table *s, char *name);
struct symbol *define_builtin(struct symbol_table *s, int index, char *name);
void symbol_table_free(struct symbol_table *s);

#endif
