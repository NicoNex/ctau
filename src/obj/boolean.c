#include <stdlib.h>
#include "obj.h"

static void dispose_boolean(struct object *o) {
	free(o);
}

struct object *parse_bool(int b) {
	return b ? true_obj : false_obj;
}

void dummy_dispose(struct object *o) {}

struct object *new_boolean_obj(int b) {
	struct object *o = calloc(1, sizeof(struct object));
	o->data.i = b != 0;
	o->type = obj_boolean;
	o->dispose = dispose_boolean;

	return o;
}

