#include <stdio.h>
#include <stdlib.h>
#include "obj.h"

static void dispose_integer_obj(struct object *o) {
	free(o);
}

static void print_integer_obj(struct object *o) {
	int64_t i = o->data.i;
	printf("%ld\n", i);
}

struct object *new_integer_obj(uint64_t val) {
	struct object *o = calloc(1, sizeof(struct object));
	o->data.i = val;
	o->type = obj_integer;
	o->dispose = dispose_integer_obj;
	o->print = print_integer_obj;

	return o;
}
