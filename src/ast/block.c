#include "ast.h"

int compile_block(struct node *n, struct compiler *c) {
	struct block_node *b = n->data;
	int pos = 0;

	for (int i = 0; i < b->len; i++) {
		struct node *expr = b->nodes[i];
		CHECK(pos = expr->compile(expr, c));

		if (expr->type != return_node_t) {
			pos = compiler_emit(c, op_pop);
		}
	}

	return pos;
}

void dispose_block_node(struct node *n) {
	struct block_node *b = n->data;

	for (int i = 0; i < b->len; i++) {
		b->nodes[i]->dispose(b->nodes[i]);
	}

	free(b);
	free(n);
}

void block_add_statement(struct block_node *b, struct node *s) {
	b->nodes = realloc(b->nodes, sizeof(struct node *) * ++b->len);
	b->nodes[b->len-1] = s;
}

struct node *new_block() {
	struct block *b = calloc(1, sizeof(struct block_node));
	return new_node(b, block_node_t, compile_block, dispose_block_node);
}

