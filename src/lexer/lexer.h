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
	void (*state)(struct lexer *l);
};

typedef void (*statefn)(struct lexer *l);

struct lexer new_lexer(char *input, size_t len);
void lexer_run(struct lexer *l);

#endif

