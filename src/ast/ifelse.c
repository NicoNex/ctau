#include "ast.h"

struct ifelse {
	struct node *cond;
	struct node *body;
	struct node *altern;
};

int compile_ifelse(struct node *n, struct compiler *c) {
	return -1;
}

void dispose_ifelse(struct node *n) {
	struct ifelse *i = n->data;
	if (i->cond != NULL) i->cond->dispose(i->cond);
	if (i->body != NULL) i->body->dispose(i->body);
	if (i->altern != NULL) i->altern->dispose(i->altern);

	free(i);
	free(n);
}

struct node *new_ifelse(struct node *cond, struct node *body, struct node *altern) {
	struct ifelse *i = malloc(sizeof(struct ifelse));
	i->cond = cond;
	i->body = body;
	i->altern = altern;

	return new_node(i, ifelse_node, compile_ifelse, dispose_ifelse);
}
