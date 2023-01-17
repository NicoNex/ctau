#include <stdint.h>
#include "greatest.h"
#include "../src/code/code.h"
#include "../src/compiler/compiler.h"

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
	free(c);

	PASS();
}

SUITE(tautest) {
	RUN_TEST(test_make);
	RUN_TEST(test_compiler);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
	GREATEST_MAIN_BEGIN();
	RUN_SUITE(tautest);
	GREATEST_MAIN_END();
}