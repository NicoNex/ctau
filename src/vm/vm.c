#include <stdarg.h>
#include <stdio.h>

#include "vm.h"
#include "../obj/obj.h"
#include "../code/code.h"

#define vm_current_frame(vm) (&vm->frames[vm->frame_idx])
#define vm_push_frame(vm, frame) vm->frames[++vm->frame_idx] = frame
#define vm_pop_frame(vm) (&vm->frames[vm->frame_idx--])

#define vm_stack_push(vm, obj) vm->stack[vm->sp++] = obj
#define vm_stack_pop(vm) (vm->stack[--vm->sp])
#define vm_stack_pop_ignore(vm) vm->sp--
#define vm_stack_peek(vm) (vm->stack[vm->sp-1])

#define DISPATCH() goto *jump_table[*frame->ip++]
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
	vm->state.consts = bytecode.consts;

	struct object *fn = new_function_obj(bytecode.insts, bytecode.len, 0, 0);
	struct object *cl = new_closure_obj(fn->data.fn, NULL, 0);
	vm->frames[0] = new_frame(cl, 0);

	return vm;
}

static inline void vm_push_closure(struct vm *restrict vm, uint32_t const_idx, uint32_t num_free) {
	struct object *cnst = vm->state.consts[const_idx];

	if (cnst->type != obj_function) {
		printf("vm_push_closure: expected closure, but got %d\n", cnst->type);
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

static inline int assert(struct object *o, size_t n, ...) {
	va_list ptr;
	va_start(ptr, n);
	register enum obj_type type = o->type;

	for (int i = 0; i < n; i++) {
		if (type == va_arg(ptr, int)) {
			return 1;
		}
	}
	va_end(ptr);
	return 0;
}

static inline void vm_exec_add(struct vm * restrict vm) {
	struct object *right = unwrap(vm_stack_pop(vm));
	struct object *left = unwrap(vm_stack_pop(vm));

	if (assert(left, 1, obj_integer) && assert(right, 1, obj_integer)) {
		vm_stack_push(vm, new_integer_obj(left->data.i + right->data.i));
	} else if (assert(left, 2, obj_integer, obj_float) && assert(right, 2, obj_integer, obj_float)) {
		puts("adding two floats is not yet supported!");
		exit(1);
		vm_stack_push(vm, new_float_obj(left->data.f + right->data.f));
	} else if (assert(left, 1, obj_string) && assert(right, 1, obj_string)) {
		puts("adding two strings is not yet supported!");
		exit(1);
	}

	puts("unsupported operator '+' for the two types");
	exit(1);
}

static inline void vm_exec_sub(struct vm * restrict vm) {
	struct object *right = unwrap(vm_stack_pop(vm));
	struct object *left = unwrap(vm_stack_pop(vm));

	if (assert(left, 1, obj_integer) && assert(right, 1, obj_integer)) {
		vm_stack_push(vm, new_integer_obj(left->data.i - right->data.i));
	} else if (assert(left, 2, obj_integer, obj_float)  && assert(right, 2, obj_integer, obj_float)) {
		puts("subtracting two floats is not yet supported");
		exit(1);
		vm_stack_push(vm, new_float_obj(left->data.f - right->data.f));
	} else if (assert(left, 1, obj_string) && assert(right, 1, obj_string)) {
		puts("subtracting two strings is not yet supported");
		exit(1);
	}

	puts("unsupported operator '-' for the two types");
	exit(1);
}

static inline void vm_exec_greater_than(struct vm * restrict vm) {
	struct object *right = unwrap(vm_stack_pop(vm));
	struct object *left = unwrap(vm_stack_pop(vm));

	if (assert(left, 1, obj_integer) && assert(right, 1, obj_integer)) {
		vm_stack_push(vm, parse_bool(left->data.i > right->data.i));
	} else if (assert(left, 2, obj_integer, obj_float) && assert(right, 2, obj_integer, obj_float)) {
		puts("comparing two floats is not yet supported");
		exit(1);
		vm_stack_push(vm, parse_bool(left->data.f > right->data.f));
	} else if (assert(left, 1, obj_string) && assert(right, 1, obj_string)) {
		puts("comparing two strings is not yet supported");
		exit(1);
	}

	puts("unsupported operator '>' for the two types");
	exit(1);
}

static inline void vm_call_closure(struct vm * restrict vm, struct object *cl, size_t numargs) {
	int num_params = cl->data.cl->fn->num_params;

	if (num_params != numargs) {
		printf("wrong number of arguments: expected %d, got %lu\n", num_params, numargs);
		exit(1);
	}

	struct frame frame = new_frame(cl, vm->sp-numargs);
	vm_push_frame(vm, frame);
	vm->sp = frame.base_ptr + cl->data.cl->fn->num_locals;
}

static inline void vm_exec_call(struct vm * restrict vm, size_t numargs) {
	struct object *o = unwrap(vm->stack[vm->sp-1-numargs]);

	switch (o->type) {
	case obj_closure:
		return vm_call_closure(vm, o, numargs);
	case obj_builtin:
		puts("calling builtins is not yet supported");
		exit(1);
	default:
		puts("calling non-function");
		exit(1);
	}
}

static inline void vm_exec_return(struct vm * restrict vm) {
	struct frame *frame = vm_pop_frame(vm);
	vm->sp = frame->base_ptr - 1;
	vm_stack_push(vm, null_obj);
}

static inline void vm_exec_return_value(struct vm * restrict vm) {
	struct object *o = unwrap(vm_stack_pop(vm));
	struct frame *frame = vm_pop_frame(vm);
	vm->sp = frame->base_ptr - 1;
	vm_stack_push(vm, o);
}

int vm_run(struct vm * restrict vm) {
#include "jump_table.h"

	struct frame *frame = vm_current_frame(vm);
	DISPATCH();

	TARGET_CONST: {
		puts("TARGET_CONST");
		uint16_t idx = read_uint16(frame->ip);
		frame->ip += 2;
		vm_stack_push(vm, vm->state.consts[idx]);
		DISPATCH();
	}

	TARGET_TRUE: {
		vm_stack_push(vm, true_obj);
		DISPATCH();
	}

	TARGET_FALSE: {
		vm_stack_push(vm, false_obj);
		DISPATCH();
	}

	TARGET_NULL: {
		vm_stack_push(vm, null_obj);
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
		puts("TARGET_CLOSURE");
		uint16_t const_idx = read_uint16(frame->ip);
		uint8_t num_free = read_uint8(frame->ip+2);
		frame->ip += 3;
		vm_push_closure(vm, const_idx, num_free);
		DISPATCH();
	}

	TARGET_CURRENT_CLOSURE: {
		puts("TARGET_CURRENT_CLOSURE");
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
		puts("TARGET_CALL");
		uint8_t num_args = read_uint8(frame->ip++);
		vm_exec_call(vm, num_args);
		frame = vm_current_frame(vm);
		DISPATCH();
	}

	TARGET_CONCURRENT_CALL: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_RETURN: {
		vm_exec_return(vm);
		frame = vm_current_frame(vm);
		DISPATCH();
	}

	TARGET_RETURN_VALUE: {
		vm_exec_return_value(vm);
		frame = vm_current_frame(vm);
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
		UNHANDLED();
		DISPATCH();
	}

	TARGET_GET_GLOBAL: {
		puts("TARGET_GET_GLOBAL");
		int global_idx = read_uint16(frame->ip);
		frame->ip += 2;
		vm_stack_push(vm, vm->state.globals[global_idx]);
		DISPATCH();
	}

	TARGET_SET_GLOBAL: {
		puts("TARGET_SET_GLOBAL");
		int global_idx = read_uint16(frame->ip);
		frame->ip += 2;
		vm->state.globals[global_idx] = vm_stack_peek(vm);
		DISPATCH();
	}

	TARGET_GET_LOCAL: {
		puts("TARGET_GET_LOCAL");
		int local_idx = read_uint8(frame->ip++);
		vm_stack_push(vm, vm->stack[frame->base_ptr+local_idx]);
		DISPATCH();
	}

	TARGET_SET_LOCAL: {
		puts("TARGET_SET_LOCAL");
		int local_idx = read_uint8(frame->ip++);
		vm->stack[frame->base_ptr+local_idx] = vm_stack_peek(vm);
		DISPATCH();
	}

	TARGET_GET_BUILTIN: {
		UNHANDLED();
		DISPATCH();
	}

	TARGET_GET_FREE: {
		int free_idx = read_uint8(frame->ip++);
		struct object *cl = frame->cl;
		vm_stack_push(vm, cl->data.cl->free[free_idx]);
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
