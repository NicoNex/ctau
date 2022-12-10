#ifndef CODE_H_
#define CODE_H_

#include <stdint.h>
#include <stddef.h>

enum opcode {
	op_constant,
	op_true,
	op_false,
	op_null,
	op_list,
	op_map,
	op_closure,
	op_current_closure,

	op_add,
	op_sub,
	op_mul,
	op_div,
	op_mod,

	op_bw_and,
	op_bw_or,
	op_bw_xor,
	op_bw_not,
	op_bw_lshift,
	op_bw_rshift,

	op_and,
	op_or,
	op_equal,
	op_not_equal,
	op_greater_than,
	op_greater_than_equal,

	op_minus,
	op_bang,
	op_index,

	op_call,
	op_concurrent_call,
	op_return,
	op_return_value,

	op_jump,
	op_jump_not_truthy,

	op_dot,
	op_define,
	op_get_global,
	op_set_global,
	op_get_local,
	op_set_local,
	op_get_builtin,
	op_get_free,
	op_load_module,
	op_interpolate,

	op_pop
};

struct definition {
	char *name;
	int *opwidths;
	int noperands;
};

struct definition definitions[op_pop+1] = {
	{"op_constant", (int[1]) {2}, 1},
	{"op_true", (int[1]) {0}, 0},
	{"op_false", (int[1]) {0}, 0},
	{"op_null", (int[1]) {0}, 0},
	{"op_list", (int[1]) {2}, 1},
	{"op_map", (int[1]) {2}, 1},
	{"op_closure", (int[2]) {2, 1}, 2},
	{"op_current_closure", (int[1]) {0}, 0},

	{"op_add", (int[1]) {0}, 0},
	{"op_sub", (int[1]) {0}, 0},
	{"op_mul", (int[1]) {0}, 0},
	{"op_div", (int[1]) {0}, 0},
	{"op_mod", (int[1]) {0}, 0},

	{"op_bw_and", (int[1]) {0}, 0},
	{"op_bw_or", (int[1]) {0}, 0},
	{"op_bw_xor", (int[1]) {0}, 0},
	{"op_bw_not", (int[1]) {0}, 0},
	{"op_bw_lshift", (int[1]) {0}, 0},
	{"op_bw_rshift", (int[1]) {0}, 0},

	{"op_and", (int[1]) {0}, 0},
	{"op_or", (int[1]) {0}, 0},
	{"op_equal", (int[1]) {0}, 0},
	{"op_not_equal", (int[1]) {0}, 0},
	{"op_greater_than", (int[1]) {0}, 0},
	{"op_greater_than_equal", (int[1]) {0}, 0},

	{"op_minus", (int[1]) {0}, 0},
	{"op_bang", (int[1]) {0}, 0},
	{"op_index", (int[1]) {0}, 0},

	{"op_call", (int[1]) {1}, 1},
	{"op_concurrent_call", (int[1]) {1}, 1},
	{"op_return", (int[1]) {0}, 0},
	{"op_return_value", (int[1]) {0}, 0},

	{"op_jump", (int[1]) {2}, 1},
	{"op_jump_not_truthy", (int[1]) {2}, 1},

	{"op_dot", (int[1]) {0}, 0},
	{"op_define", (int[1]) {0}, 0},
	{"op_get_global", (int[1]) {2}, 1},
	{"op_set_global", (int[1]) {2}, 1},
	{"op_get_local", (int[1]) {1}, 1},
	{"op_set_local", (int[1]) {1}, 1},
	{"op_get_builtin", (int[1]) {1}, 1},
	{"op_get_free", (int[1]) {1}, 1},
	{"op_load_module", (int[1]) {0}, 0},
	{"op_interpolate", (int[2]) {2, 2}, 2},

	{"op_pop", (int[1]) {0}, 0}
};

int lookup_def(enum opcode op, struct definition *def);
size_t make_bcode(uint8_t **code, size_t code_len, enum opcode op, ...);
int read_operands(struct definition def, uint8_t *ins, int **operands);

uint8_t read_uint8(uint8_t *ins);
uint16_t read_uint16(uint8_t *ins);
uint32_t read_uint32(uint8_t *ins);

#endif

