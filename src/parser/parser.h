#ifndef PARSER_H_
#define PARSER_H_

#include <stdint.h>

#include "../item/item.h"
#include "../ast/ast.h"

struct parser {
	struct item *items;
	size_t nitems;
	int index;
	struct item cur;
	struct item peek;
	uint nested_loops;
};

typedef struct node *(*prefixfn)(struct parser *p);
typedef struct node *(*infixfn)(struct parser *p, struct node *n);

struct node *parse_input(char *input, size_t len);

#endif
