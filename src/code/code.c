#include <stdlib.h>
#include "code.h"

#define NUM_OPCODES 45

int lookup_def(enum opcode opcode, struct definition *def) {
	if (opcode >= NUM_OPCODES) {
		return 0;
	}

	*def = definitions[opcode];
	return 1;
}

static inline void put_uint16(uint8_t *code, uint16_t i) {
	code[0] = (i & (0xff << 8)) >> 8;
	code[1] = i & 0xff;
}

static inline void put_uint32(uint8_t *code, uint32_t i) {
	code[0] = (i & (0xff << 24)) >> 24;
	code[1] = (i & (0xff << 16)) >> 16;
	code[2] = (i & (0xff << 8)) >> 8;
	code[3] = i & 0xff;
}

// Returns the resulting bytecode from parsing the instruction.
size_t make_bcode(uint8_t **code, size_t code_len, enum opcode op, ...) {
	struct definition def;

	if (!lookup_def(op, &def)) {
		return code_len;
	}

	int ins_len = 1;

	for (int i = 0; i < def.noperands; i++) {
		ins_len += def.opwidths[i];
	}

	int offset = code_len;
	code_len += sizeof(uint8_t) * ins_len;
	*code = realloc(*code, code_len);
	uint8_t *bcode = *code;
	bcode[offset++] = op;

	va_list operands;
	va_start(operands, op);

	for (int i = 0; i < def.noperands; i++) {
		int width = def.opwidths[i];

		switch (width) {
			case 1: {
				uint8_t operand = va_arg(operands, uint32_t);
				bcode[offset] = operand;
				break;
			}

			case 2: {
				uint16_t operand = va_arg(operands, uint32_t);
				put_uint16(&bcode[offset], operand);
				break;
			}

			case 4: {
				uint32_t operand = va_arg(operands, uint32_t);
				put_uint32(&bcode[offset], operand);
				break;
			}
		}

		offset += width;
	}

	va_end(operands);
	return code_len;
}

size_t vmake_bcode(uint8_t **code, size_t code_len, enum opcode op, va_list operands) {
	struct definition def;

	if (!lookup_def(op, &def)) {
		return code_len;
	}

	int ins_len = 1;

	for (int i = 0; i < def.noperands; i++) {
		ins_len += def.opwidths[i];
	}

	int offset = code_len;
	code_len += sizeof(uint8_t) * ins_len;
	*code = realloc(*code, code_len);
	uint8_t *bcode = *code;
	bcode[offset++] = op;

	for (int i = 0; i < def.noperands; i++) {
		int width = def.opwidths[i];

		switch (width) {
			case 1: {
				uint8_t operand = va_arg(operands, uint32_t);
				bcode[offset] = operand;
				break;
			}

			case 2: {
				uint16_t operand = va_arg(operands, uint32_t);
				put_uint16(&bcode[offset], operand);
				break;
			}

			case 4: {
				uint32_t operand = va_arg(operands, uint32_t);
				put_uint32(&bcode[offset], operand);
				break;
			}
		}

		offset += width;
	}

	return code_len;
}

inline uint8_t read_uint8(uint8_t *ins) {
	return ins[0];
}

inline uint16_t read_uint16(uint8_t *ins) {
	return (ins[0] << 8) | ins[1];
}

inline uint32_t read_uint32(uint8_t *ins) {
	return (ins[0] << 24) | (ins[1] << 16) | (ins[2] << 8) | ins[3];
}

// Decodes the operands of a bytecode instruction.
int read_operands(struct definition def, uint8_t *ins, int **operands) {
	*operands = realloc(*operands, sizeof(int) * def.noperands);
	int offset = 0;

	for (int i = 0; i < def.noperands; i++) {
		int width = def.opwidths[i];

		switch (width) {
		case 1:
			*operands[i] = read_uint8(&ins[offset]);
		case 2:
			*operands[i] = read_uint16(&ins[offset]);
		case 3:
			*operands[i] = read_uint32(&ins[offset]);
		}

		offset += width;
	}

	return offset;
}

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
