#include <stdio.h>

#include "vm.h"
#include "../obj/obj.h"
#include "../code/code.h"
#include "jump_table.h"

#define DISPATCH() goto *jump_table[*frame->ip]

#define vm_current_frame(vm) (&vm->frames[vm->frame_idx])
#define vm_stack_push(vm, obj) vm->stack[vm->sp++] = obj
#define vm_stack_pop(vm) vm->stack[--vm->sp]

#define UNHANDLED() puts("unhandled opcode"); return -1

struct frame new_frame(struct object *cl, uint32_t base_ptr) {
	return (struct frame) {
		.cl = cl,
		.base_ptr = base_ptr
	};
}

struct state new_state() {
	return (struct state) {0};
}

struct vm *new_vm(struct bytecode bytecode) {
	struct vm *vm = calloc(1, sizeof(struct vm));
	vm->frame_idx = 1;
	vm->state.consts = bytecode.consts;

	struct object *fn = new_function_obj(bytecode.insts, bytecode.ninsts, 0, 0);
	struct object *cl = new_closure_obj(fn, NULL, 0);
	vm->frames[0] = new_frame(cl, 0);

	return vm;
}

/*
#define vm_stack_pop_ignore(vm) vm->stack_pointer -= 1;
#define vm_stack_pop(vm) (vm->stack->values[--vm->stack_pointer])
#define vm_stack_cur(vm) (vm->stack->values[vm->stack_pointer - 1])
#define vm_stack_push(vm, obj) vm->stack = insert_in_object_list(vm->stack, vm->stack_pointer++, obj);
*/

static inline void vm_push_closure(struct vm *restrict vm, uint32_t const_idx, uint32_t num_free) {
	struct object *cnst = vm->state.consts[const_idx];

	if (cnst->type != obj_function) {
		puts("vm_push_closure: expected closure");
		exit(1);
	}
	
	struct object **free = malloc(sizeof(struct object *) * num_free);
	for (int i = 0; i < num_free; i++) {
		free[i] = vm->stack[vm->sp-num_free+i];
	}

	struct object *cl = new_closure_obj(cnst->data.fn, free, num_free);
	vm->sp -= num_free;
	vm_stack_push(vm, cl);
}

int vm_run(struct vm * restrict vm) {
	struct frame *frame = vm_current_frame(vm);
	DISPATCH();

	TARGET_CONST: {
		uint16_t idx = read_uint16(frame->ip + 1);
		frame->ip += 3;
		vm_stack_push(vm, vm->state.consts[idx]);
		DISPATCH();
	}

	TARGET_TRUE: {
		vm_stack_push(vm, true_obj);
		frame->ip++;
		DISPATCH();
	}

	TARGET_FALSE: {
		vm_stack_push(vm, false_obj);
		frame->ip++;
		DISPATCH();
	}

	TARGET_NULL: {
		vm_stack_push(vm, null_obj);
		frame->ip++;
		DISPATCH();
	}

	TARGET_LIST: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_MAP: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_CLOSURE: {
		uint16_t const_idx = read_uint16(frame->ip+1);
		uint8_t num_free = read_uint8(frame->ip+3);
		frame->ip += 3;
		vm_push_closure(vm, const_idx, num_free);
		DISPATCH();
	}

	TARGET_CURRENT_CLOSURE: {
		vm_stack_push(vm, frame->cl);
		DISPATCH();
	}


	TARGET_ADD: {

		DISPATCH();
	}

	TARGET_SUB: {

		DISPATCH();
	}

	TARGET_MUL: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_DIV: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_MOD: {
		UNHANDLED();
		DISPATCH();
	}


	TARGET_BW_AND: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_BW_OR: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_BW_XOR: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_BW_NOT: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_BW_LSHIFT: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_BW_RSHIFT: {
		UNHANDLED();
		DISPATCH();
	}


	TARGET_AND: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_OR: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_EQUAL: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_NOT_EQUAL: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_GREATER_THAN: {

		DISPATCH();
	}

	TARGET_GREATER_THAN_EQUAL: {

		DISPATCH();
	}


	TARGET_MINUS: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_BANG: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_INDEX: {
		UNHANDLED();
		DISPATCH();
	}


	TARGET_CALL: {

		DISPATCH();
	}

	TARGET_CONCURRENT_CALL: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_RETURN: {

		DISPATCH();
	}

	TARGET_RETURN_VALUE: {

		DISPATCH();
	}


	TARGET_JUMP: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_JUMP_NOT_TRUTHY: {
		UNHANDLED();
		DISPATCH();
	}


	TARGET_DOT: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_DEFINE: {

		DISPATCH();
	}

	TARGET_GET_GLOBAL: {

		DISPATCH();
	}

	TARGET_SET_GLOBAL: {

		DISPATCH();
	}

	TARGET_GET_LOCAL: {

		DISPATCH();
	}

	TARGET_SET_LOCAL: {

		DISPATCH();
	}

	TARGET_GET_BUILTIN: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_GET_FREE: {

		DISPATCH();
	}

	TARGET_LOAD_MODULE: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_INTERPOLATE: {
		UNHANDLED();
		DISPATCH();
	}


	TARGET_POP: {
		vm_stack_pop(vm);
		DISPATCH();
	}
}
