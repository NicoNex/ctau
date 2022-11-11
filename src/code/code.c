#include "code.h"

inline struct definition get_def(enum opcode opcode) {
    return definitions[opcode];
}
