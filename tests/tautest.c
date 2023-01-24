#include <string.h>
#include <stdint.h>
#include "greatest.h"
#include "../src/code/code.h"
#include "../src/compiler/compiler.h"
#include "../src/data/map.h"

#define RESET_CODE(code) free(code); code = NULL

int compare(size_t n, uint8_t *b1, uint8_t *b2) {
	for (int i = 0; i < n; i++) {
		if (b1[i] != b2[i]) {
			return 0;
		}
	}
	return 1;
}

TEST test_make(void) {
	uint8_t *code = NULL;
	uint8_t *expected = NULL;

	expected = (uint8_t[3]) {op_constant, 255, 254};
	make_bcode(&code, 0, op_constant, 65534);
	ASSERT(compare(3, code, expected));
	RESET_CODE(code);

	expected = (uint8_t[1]) {op_greater_than};
	make_bcode(&code, 0, op_greater_than);
	ASSERT(compare(1, code, expected));
	RESET_CODE(code);

	expected = (uint8_t[1]) {op_add};
	make_bcode(&code, 0, op_add);
	ASSERT(compare(1, code, expected));
	RESET_CODE(code);

	expected = (uint8_t[4]) {op_closure, 255, 254, 255};
	make_bcode(&code, 0, op_closure, 65534, 255);
	ASSERT(compare(4, code, expected));
	RESET_CODE(code);

	PASS();
}

TEST test_compiler(void) {
	struct compiler *c = NULL;
	struct bytecode bc;

	c = new_compiler();
	int pos = compiler_add_const(c, new_integer_obj(255));
	compiler_emit(c, op_constant, pos);
	compiler_emit(c, op_halt);
	bc = compiler_bytecode(c);
	ASSERT(bc.len == 4);
	ASSERT(bc.insts[0] == op_constant);
	ASSERT(read_uint16(&bc.insts[1]) == pos);
	ASSERT(bc.insts[3] == op_halt);
	free(c);

	c = new_compiler();
	int jump_pos = compiler_emit(c, op_jump, 9999);
	compiler_replace_operand(c, jump_pos, 123);
	bc = compiler_bytecode(c);
	ASSERT(bc.len == 3);
	ASSERT(bc.insts[0] == op_jump);
	ASSERT(read_uint16(&bc.insts[1]) == 123);
	free(c);

	c = new_compiler();
	pos = compiler_add_const(c, new_integer_obj(123));
	int pos2 = compiler_add_const(c, new_integer_obj(456));
	compiler_emit(c, op_constant, pos);
	bc = compiler_bytecode(c);
	ASSERT(bc.len = 3);
	ASSERT(bc.insts[0] == op_constant);
	ASSERT(read_uint16(&bc.insts[1]) == pos);
	struct object *o1 = bc.consts[pos];
	struct object *o2 = bc.consts[pos2];
	ASSERT(o1 != NULL);
	ASSERT(o1->data.i == 123);
	ASSERT(o2 != NULL);
	ASSERT(o2->data.i == 456);
	free(c);
	free(o1);

	PASS();
}

TEST test_symboltable(void) {
	struct symbol_table *outer = new_symbol_table();
	struct symbol_table *st = new_enclosed_symbol_table(outer);

	struct symbol *s1 = symbol_table_define(outer, "test1");
	struct symbol *s2 = symbol_table_define(st, "test2");
	struct symbol *t1 = symbol_table_resolve(st, "test1");
	struct symbol *t2 = symbol_table_resolve(st, "test2");

	ASSERT(t1->scope == global_scope);
	ASSERT(t2->scope == local_scope);

	ASSERT(strcmp(s1->name, t1->name) == 0);
	ASSERT(s1->scope == t1->scope);
	ASSERT(s1->index == t1->index);

	ASSERT(strcmp(s2->name, t2->name) == 0);
	ASSERT(s2->scope == t2->scope);
	ASSERT(s2->index == t2->index);

	// symbol_table_free(outer);
	// symbol_table_free(st);

	PASS();
}

SUITE(tautest) {
	RUN_TEST(test_make);
	RUN_TEST(test_compiler);
	RUN_TEST(test_symboltable);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
	GREATEST_MAIN_BEGIN();
	RUN_SUITE(tautest);
	GREATEST_MAIN_END();
}
