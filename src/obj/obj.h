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

typedef struct object object;

union data {
	uint64_t i;
	double f;
	char *str;
	struct object *list;
};

struct object {
	union data data;
	enum obj_type type;
    size_t len;
	void (*dispose)(struct object *o);
};

// typedef void (*disposefn)(struct object *o);

#endif
