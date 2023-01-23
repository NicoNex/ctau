#include <stdlib.h>
#include "compiler.h"

struct symbol *new_symbol(char *name, enum symbol_scope scope, int index) {
	struct symbol *s = calloc(1, sizeof(struct symbol));
	s->name = name;
	s->scope = scope;
	s->index = index;

	return s;
}

void symbol_free(struct symbol *s) {
	free(s->name);
	free(s);
}

struct symbol *symbol_table_define(struct symbol_table *s, char *name) {
	struct symbol *symbol = strmap_get(s->store, name);

	if (symbol!= NULL) {
		return symbol;
	}

	enum symbol_scope scope = s->outer != NULL ? local_scope : global_scope;
	symbol = new_symbol(name, scope, s->num_defs);
	strmap_set(&s->store, name, symbol);
	s->num_defs++;
	return symbol;
}

struct symbol *symbol_table_define_free(struct symbol_table *s, struct symbol *original) {
	struct symbol *symbol = new_symbol(original->name, s->nfree, free_scope);

	s->free_symbols = realloc(s->free_symbols, sizeof(struct symbol *) * ++s->nfree);
	s->free_symbols[s->nfree-1] = symbol;
	return symbol;
}

struct symbol *symbol_table_resolve(struct symbol_table *s, char *name) {
	struct symbol *symbol = strmap_get(s->store, name);

	if (symbol != NULL) {
		return symbol;
	}

	if (s->outer != NULL) {
		symbol = symbol_table_resolve(s->outer, name);

		if (symbol != NULL) {
			if (symbol->scope == global_scope || symbol->scope == builtin_scope) {
				return symbol;
			}

			return symbol_table_define_free(s, symbol);
		}
	}

	return NULL;
}

struct symbol *define_builtin(struct symbol_table *s, int index, char *name) {
	struct symbol *symbol = new_symbol(name, index, builtin_scope);
	strmap_set(&s->store, name, symbol);
	
	return symbol;
}

struct symbol_table *new_symbol_table() {
	return calloc(1, sizeof(struct symbol_table));
}

struct symbol_table *new_enclosed_symbol_table(struct symbol_table *outer) {
	struct symbol_table *s = new_symbol_table();
	s->outer = outer;

	return s;
}

static void _strmap_symbol_free(char *key, void *val) {
	free(key);
	free(val);
}

void symbol_table_free(struct symbol_table *s) {
	strmap_free_fn(s->store, _strmap_symbol_free);

	for (int i = 0; i < s->nfree; i++) {
		symbol_free(s->free_symbols[i]);
	}

	free(s->free_symbols);
	free(s);
}

