#include "obj.h"

static dispose_boolean(struct object *o) {
	free(o);
}

struct object *new_boolean(int b) {
	struct object *o = calloc(1, sizeof(struct object));
	o->data.i = b != 0;
	o->type = obj_boolean;
	o->dispose = dispose_boolean;

	return o;
}
