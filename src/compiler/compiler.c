#include "compiler.h"

struct compiler new_compiler() {
	return {
		.constants = NULL,
		.ninstructions = 0,

	};
}
