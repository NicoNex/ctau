#ifndef COMPILER_H_
#define COMPILER_H_

#include <stdint.h>

#include "../ast/ast.h"
#include "../obj/obj.h"
#include "../code/code.h"

struct emitted_inst {
    enum opcode opcode;
    int position;
};

struct compilation_scope {
    uint8_t *instructions;
    size_t ninstructions;
    struct emitted_inst last_inst;
    struct emitted_inst prev_inst;
};

struct compiler {
    struct obj *constants;
    size_t nconstants;
    struct compilation_scope *scopes;
    size_t nscopes;
    int scope_index;
};

struct bytecode {
    uint8_t *instructions;
    size_t ninstructions;
    struct obj *constants;
    size_t nconstants;
};

#endif
