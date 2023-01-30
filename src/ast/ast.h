#ifndef AST_H_
#define AST_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../compiler/compiler.h"

#define CHECK(pos) if ((pos) == -1) return -1

struct compiler;

enum node_type {
	plus_node_t,
	minus_node_t,
	integer_node_t,
	less_node_t,
	lesseq_node_t,
	greater_node_t,
	greatereq_node_t,
	block_node_t,
	function_node_t,
	call_node_t,
	null_node_t,
	return_node_t,
	identifier_node_t,
	ifelse_node_t,
	assign_node_t
};

struct node {
	void *data;
	enum node_type type;
	int (*compile)(struct node *n, struct compiler *c);
	void (*dispose)(struct node *n);
};

typedef int (*compilefn)(struct node *n, struct compiler *c);
typedef void (*disposefn)(struct node *n);

struct block_node {
	struct node **nodes;
	size_t len;
};

struct node *new_node(void *data, enum node_type t, compilefn cfn, disposefn dfn);
struct node *new_plus(struct node *l, struct node *r);
struct node *new_minus(struct node *l, struct node *r);
struct node *new_block();
struct node *new_null();
struct node *new_return(struct node *val);
struct node *new_identifier(char *name);
struct node *new_integer(int64_t *val);
struct node *new_less(struct node *l, struct node *r);
struct node *new_less_eq(struct node *l, struct node *r);
struct node *new_greater(struct node *l, struct node *r);
struct node *new_greater_eq(struct node *l, struct node *r);
struct node *new_assign(struct node *l, struct node *r);
struct node *new_call(struct node *fn, struct node **args, size_t arglen);
struct node *new_ifelse(struct node *cond, struct node *body, struct node *altern);
struct node *new_function(char **params, size_t nparams, struct node *body);

void block_add_statement(struct block_node *b, struct node *s);

#endif

