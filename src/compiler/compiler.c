#include <stdlib.h>
#include "compiler.h"

struct compiler *new_compiler() {
	return calloc(1, sizeof(struct compiler));
}
