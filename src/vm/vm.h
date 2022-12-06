#ifndef VM_H_
#define VM_H_

#include <stdint.h>
#include "../obj/obj.h"
#include "../compiler/compiler.h"

#define STACK_SIZE 2048
#define GLOBAL_SIZE 65536
#define MAX_FRAMES 1024

struct frame {
	struct object *closure;
	uint8_t *ip;
	uint32_t base_ptr;
};

struct state {
	struct symbol_table *st;
	struct object **consts;
	struct object *globals[GLOBAL_SIZE];
};

struct vm {
	struct object *stack[STACK_SIZE];
	struct frame frames[MAX_FRAMES];
	struct state state;
	uint32_t sp;
	uint32_t frame_idx;
};

struct frame new_frame(struct closure cl, uint32_t base_ptr);
struct vm *new_vm(struct bytecode bytecode);

#endif
