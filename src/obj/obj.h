#ifndef OBJ_H_
#define OBJ_H_

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

struct object {
    void *data;
    enum obj_type type;
    void (*dispose)(struct object *o);
};

typedef void (*disposefn)(struct object *o);

void obj_to_string(struct object *o, char *str);

#endif
