#ifndef MAP_H_
#define MAP_H_

#include <stdint.h>

typedef struct node {
	uint64_t hash;
	char *key;
	void *val;
	struct node *l;
	struct node *r;
} *strmap;

void strmap_set(strmap *m, char *key, void *val);
void *strmap_get(strmap m, char *key);
void strmap_free(strmap m);

#endif
