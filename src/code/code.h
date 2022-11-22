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
	{"op_constant", {2}, 1},
	{"op_true", {0}, 0},
	{"op_false", {0}, 0},
	{"op_null", {0}, 0},
	{"op_list", {2}, 1},
	{"op_map", {2}, 1},
	{"op_closure", {2, 1}, 2},
	{"op_current_closure", {0}, 0},

	{"op_add", {0}, 0},
	{"op_sub", {0}, 0},
	{"op_mul", {0}, 0},
	{"op_div", {0}, 0},
	{"op_mod", {0}, 0},

	{"op_bw_and", {0}, 0},
	{"op_bw_or", {0}, 0},
	{"op_bw_xor", {0}, 0},
	{"op_bw_not", {0}, 0},
	{"op_bw_lshift", {0}, 0},
	{"op_bw_rshift", {0}, 0},

	{"op_and", {0}, 0},
	{"op_or", {0}, 0},
	{"op_equal", {0}, 0},
	{"op_not_equal", {0}, 0},
	{"op_greater_than", {0}, 0},
	{"op_greater_than_equal", {0}, 0},

	{"op_minus", {0}, 0},
	{"op_bang", {0}, 0},
	{"op_index", {0}, 0},

	{"op_call", {1}, 1},
	{"op_concurrent_call", {1}, 1},
	{"op_return", {0}, 0},
	{"op_return_value", {0}, 0},

	{"op_jump", {2}, 1},
	{"op_jump_not_truthy", {2}, 1},

	{"op_dot", {0}, 0},
	{"op_define", {0}, 0},
	{"op_get_global", {2}, 1},
	{"op_set_global", {2}, 1},
	{"op_get_local", {1}, 1},
	{"op_set_local", {1}, 1},
	{"op_get_builtin", {1}, 1},
	{"op_get_free", {1}, 1},
	{"op_load_module", {0}, 0},
	{"op_interpolate", {2, 2}, 2},

	{"op_pop", {0}, 0}
};

int lookup_def(enum opcode op, struct definition *def);
size_t make_bcode(uint8_t **code, size_t code_len, enum opcode op, ...);
int read_operands(struct definition def, uint8_t *ins, int **operands);

uint8_t read_uint8(uint8_t *ins);
uint16_t read_uint16(uint8_t *ins);
uint32_t read_uint32(uint8_t *ins);

#endif

