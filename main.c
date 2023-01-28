#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "src/ast/ast.h"
#include "src/parser/parser.h"
#include "src/compiler/compiler.h"
#include "src/vm/vm.h"

#define BUF_SIZE 4096

static inline int contains(char *s, char c) {
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		if (s[i] == c) {
			return 1;
		}
	}
	return 0;
}

static void trim_right(char buf[BUF_SIZE], char *cutset) {
	size_t len = strlen(buf);

	for (int i = len; i >= 0 && buf[i] != '\0'; i--) {
		if (contains(cutset, buf[i])) {
			buf[i] = '\0';
		}
	}
}

static inline size_t accept_until(char buf[BUF_SIZE], char *end) {
	size_t len = strlen(buf);
	buf[len++] = '\n';

	for (;;) {
		printf("... ");
		fgets(&buf[len], BUF_SIZE - len, stdin);
		len += strlen(&buf[len]);

		if (strcmp(&buf[len-2], end) == 0) {
			break;
		}
	}

	return len;
}

int main() {
	struct state state = new_state();

	for (;;) {
		char buf[BUF_SIZE] = {'\0'};
		printf(">>> ");
		fgets(buf, BUF_SIZE, stdin);
		trim_right(buf, " \n\t\r");

		size_t len = strlen(buf);
		if (len > 0 && buf[len-1] == '{') {
			len = accept_until(buf, "\n\n");
		}

		struct node *tree = parse_input(buf, len);
		struct compiler *c = new_compiler_with_state(state.st, &state.consts, state.nconsts);
		compile(c, tree);
		struct bytecode bc = compiler_bytecode(c);
		state.nconsts += bc.nconsts;

		struct vm *vm = new_vm_with_state(bc, state);
		vm_run(vm);

		struct object *o = vm_last_popped_stack_elem(vm);
		o->print(o);

		tree->dispose(tree);
		compiler_dispose(c);
		vm_dispose(vm);
	}
}
