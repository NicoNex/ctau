#ifndef AST_H_
#define AST_H_

#include <stdio.h>
#include <stdlib.h>
#include "../compiler/compiler.h"

enum type {
	PlusNode,
	MinusNode,
	IntegerNode,
	LessNode,
	LessEqNode,
	GreaterNode,
	GreaterEqNode,
	FunctionNode,
	CallNode,
	ReturnNode
};

struct compiler {};

struct node {
	void *data;
	enum type type;
	int (*compile)(struct node *n, struct compiler *c);
	void (*dispose)(struct node *n);
};

typedef int (*compilefn)(struct node *n, struct compiler *c);
typedef void (*disposefn)(struct node *n);

struct node *new_node(void *data, enum type t, compilefn cfn, disposefn dfn);
struct node *new_plus(struct node *l, struct node *r);
struct node *new_minus(struct node *l, struct node *r);

#endif
