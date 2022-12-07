#include <stdio.h>

#include "vm.h"
#include "../obj/obj.h"
#include "../code/code.h"

#define DISPATCH() goto *jump_table[*frame->ip]

#define vm_current_frame(vm) (&vm->frames[vm->frame_idx])
#define vm_stack_push(vm, obj) vm->stack[vm->sp++] = obj
#define vm_stack_pop(vm) (vm->stack[--vm->sp])
#define vm_stack_pop_ignore(vm) vm->sp -= 1;
#define vm_stack_cur(vm) (vm->stack[vm->sp - 1])

#define UNHANDLED() puts("unhandled opcode"); return -1

static inline struct frame new_frame(struct object *cl, uint32_t base_ptr) {
	return (struct frame) {
		.cl = cl,
		.base_ptr = base_ptr,
		.ip = cl->data.cl->fn->instructions
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
	struct object *cl = new_closure_obj(fn->data.fn, NULL, 0);
	vm->frames[0] = new_frame(cl, 0);

	return vm;
}

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

static inline struct object *unwrap(struct object *o) {
	if (o->type == obj_getsetter) {
		// TODO: fill this.
	}
	return o;
}

static inline int assert_types(struct object *o, size_t n, ...) {
	va_list ptr;
	va_start(ptr, n);
	register enum obj_type type = o->type;

	for (int i = 0; i < n; i++) {
		if (o->type == va_arg(ptr, int)) {
			return 1;
		}
	}
	va_end(ptr);
	return 0;
}

static inline void vm_exec_add(struct vm * restrict vm) {
	struct object *right = unwrap(vm_stack_pop(vm));
	struct object *left = unwrap(vm_stack_pop(vm));

	if (assert_types(left, 1, obj_integer) && assert_types(right, 1, obj_integer)) {
		vm_stack_push(vm, new_integer_obj(left->data.i + right->data.i));
	} else if (assert_types(left, 2, obj_integer, obj_float)
			   && assert_types(right, 2, obj_integer, obj_float)) {
		puts("adding two floats is not yet supported!");
		exit(1);
		vm_stack_push(vm, new_float_obj(left->data.f + right->data.f));
	} else if (assert_types(left, 1, obj_string) && assert_types(right, 1, obj_string)) {
		puts("adding two strings is not yet supported!");
		exit(1);
	}

	puts("unsupported operator '+' for the two types");
	exit(1);
}

static inline void vm_exec_sub(struct vm * restrict vm) {
	struct object *right = unwrap(vm_stack_pop(vm));
	struct object *left = unwrap(vm_stack_pop(vm));

	if (assert_types(left, 1, obj_integer) && assert_types(right, 1, obj_integer)) {
		vm_stack_push(vm, new_integer_obj(left->data.i - right->data.i));
	} else if (assert_types(left, 2, obj_integer, obj_float)
			   && assert_types(right, 2, obj_integer, obj_float)) {
		puts("subtracting two floats is not yet supported");
		exit(1);
		vm_stack_push(vm, new_float_obj(left->data.f - right->data.f));
	} else if (assert_types(left, 1, obj_string) && assert_types(right, 1, obj_string)) {
		puts("subtracting two strings is not yet supported");
		exit(1);
	}

	puts("unsupported operator '-' for the two types");
	exit(1);
}

static inline void vm_exec_greater_than(struct vm * restrict vm) {
	struct object *right = unwrap(vm_stack_pop(vm));
	struct object *left = unwrap(vm_stack_pop(vm));

	if (assert_types(left, 1, obj_integer) && assert_types(right, 1, obj_integer)) {
		vm_stack_push(vm, parse_bool(left->data.i > right->data.i));
	} else if (assert_types(left, 2, obj_integer, obj_float)
			   && assert_types(right, 2, obj_integer, obj_float)) {
		puts("comparing two floats is not yet supported");
		exit(1);
		vm_stack_push(vm, parse_bool(left->data.f > right->data.f));
	} else if (assert_types(left, 1, obj_string) && assert_types(right, 1, obj_string)) {
		puts("comparing two strings is not yet supported");
		exit(1);
	}

	puts("unsupported operator '>' for the two types");
	exit(1);
}

int vm_run(struct vm * restrict vm) {
#include "jump_table.h"

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
		vm_exec_add(vm);
		DISPATCH();
	}

	TARGET_SUB: {
		vm_exec_sub(vm);
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
		vm_exec_greater_than(vm);
		DISPATCH();
	}

	TARGET_GREATER_THAN_EQUAL: {
		UNHANDLED();
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
		vm_stack_pop_ignore(vm);
		DISPATCH();
	}
}
