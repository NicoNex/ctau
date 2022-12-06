#ifndef OBJ_H_
#define OBJ_H_

#include <stdint.h>
#include <stddef.h>

enum obj_type {
	obj_boolean,
	obj_builtin,
	obj_bytes,
	obj_class,
	obj_closure,
	obj_error,
	obj_float,
	obj_function,
	obj_getsetter,
	obj_integer,
	obj_list,
	obj_map,
	obj_module,
	obj_nativestruct,
	obj_null,
	obj_pipe,
	obj_plugin,
	obj_string
};

struct function {
	uint8_t *instructions;
	size_t len;
	int num_locals;
	int num_params;
};

typedef struct object object;

struct closure {
	struct function *fn;
	struct object **free;
	size_t num_free;
};

union data {
	int64_t i;
	double f;
	char *str;
	struct object **list;
	struct function *fn;
	struct closure *cl;
};

struct object {
	union data data;
	enum obj_type type;
    size_t len;
	void (*dispose)(struct object *o);
};

struct object *new_function_obj(uint8_t *insts, size_t len, int num_locals, int num_params);
struct object *new_closure_obj(struct function *fn, struct object **free, size_t num_free);
struct object *new_boolean_obj(int b);
struct object *new_integer_obj(uint64_t val);
struct object *parse_bool(int b);

void dummy_dispose(struct object *o) {}

object *true_obj = &(struct object) {
    .data.i = 1,
    .type = obj_boolean,
    .len = 0,
    .dispose = dummy_dispose
};

object *false_obj = &(struct object) {
    .data.i = 0,
    .type = obj_boolean,
    .len = 0,
    .dispose = dummy_dispose
};

object *null_obj = &(struct object) {
    .data.i = 0,
    .type = obj_null,
    .len = 0,
    .dispose = dummy_dispose
};

#endif

