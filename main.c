#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "src/ast/ast.h"
#include "src/parser/parser.h"
#include "src/compiler/compiler.h"
#include "src/vm/vm.h"

int main() {
	char *code = "\
fib = fn(n) {\
	if n < 2 {\
		return n\
	}\
	fib(n-1) + fib(n-2)\
}\
fib(35)";


	struct node *tree = parse_input(code, strlen(code));
	struct compiler *c = new_compiler();
	compile(c, tree);
	struct vm *vm = new_vm(compiler_bytecode(c));
	vm_run(vm);
	struct object *o = vm_last_popped_stack_elem(vm);
	o->print(o);

	return 0;
}
