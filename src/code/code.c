#include <stdlib.h>
#include <stdarg.h>
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

	code_len += sizeof(uint8_t) * ins_len;
	*code = realloc(*code, code_len);
	int offset = code_len;

	va_list operands;
	va_start(operands, op);

	for (int i = 0; i < def.noperands; i++) {
		int width = def.opwidths[i];

		switch (width) {
			case 1: {
				uint8_t operand = va_arg(operands, uint32_t);
				*code[offset] = operand;
				break;
			}

			case 2: {
				uint16_t operand = va_arg(operands, uint32_t);
				put_uint16(&(*code[offset]), operand);
				break;
			}

			case 4: {
				uint32_t operand = va_arg(operands, uint32_t);
				put_uint32(&(*code[offset]), operand);
				break;
			}
		}

		offset += width;
	}

	va_end(operands);
	return code_len;
}

uint8_t read_uint8(uint8_t *ins) {
	return ins[0];
}

uint16_t read_uint16(uint8_t *ins) {
	return (ins[0] << 8) | ins[1];
}

uint32_t read_uint32(uint8_t *ins) {
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

