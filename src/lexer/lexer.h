#ifndef LEXER_H_
#define LEXER_H_

#include <wchar.h>
#include <stdlib.h>
#include "../item/item.h"

struct lexer {
	char *input;
	size_t len;
	int start;
	int pos;
	int width;
	int nitems;
	struct item *items;
	void (*statefn)(const struct lexer *l) state;
};

typedef void (*statefn)(const struct lexer *l);

#endif
