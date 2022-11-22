#include "obj.h"

void dispose_function(struct object *o) {
	free(o->data.fn);
	free(o);
}

struct object *new_function_obj(uint8_t *insts, size_t len, int num_locals, int num_params) {
	struct function *fn = malloc(sizeof(struct function));
	fn->instructions = insts;
	fn->len = len;
	fn->num_locals = num_locals;
	fn->num_params = num_params;

	struct object *o = calloc(1, sizeof(struct object));
	o->data.fn = fn;
	o->type = obj_function;
	o->dispose = dispose_function;

	return o;
}
