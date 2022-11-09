#ifndef AST_H_
#define AST_H_

#include <stdio.h>
#include <stdlib.h>
#include "../compiler/compiler.h"

enum node_type {
	plus_node,
	minus_node,
	integer_node,
	less_node,
	lesseq_node,
	greater_node,
	greatereq_node,
	block_node,
	function_node,
	call_node,
	null_node,
	return_node,
	identifier_node,
	ifelse_node,
	assign_node
};

struct compiler {};

struct node {
	void *data;
	enum node_type type;
	int (*compile)(struct node *n, struct compiler *c);
	void (*dispose)(struct node *n);
};

struct block {
	struct node **nodes;
	size_t len;
};

typedef int (*compilefn)(struct node *n, struct compiler *c);
typedef void (*disposefn)(struct node *n);

struct node *new_node(void *data, enum node_type t, compilefn cfn, disposefn dfn);
struct node *new_plus(struct node *l, struct node *r);
struct node *new_minus(struct node *l, struct node *r);
struct node *new_block();
struct node *new_null();
struct node *new_return(struct node *val);
struct node *new_identifier(char *name);
struct node *new_integer(int64_t *val);
struct node *new_less(struct node *l, struct node *r);
struct node *new_assign(struct node *l, struct node *r);
struct node *new_call(struct node *fn, struct node **args, size_t arglen);

void block_add_statement(struct block *b, struct node *s);

#endif
