#include "obj.h"

static dispose_integer(struct object *o) {
	free(o);
}

struct object *new_integer(uint64_t val) {
	struct object *o = calloc(1, sizeof(struct object));
	o.data.i = val;
	o.type = obj_integer;
	o.dispose = dispose_integer;

	return o;
}
