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

typedef void (*strmap_free_fn)(char *key, void *val);

void strmap_set(strmap *m, char *key, void *val);
void *strmap_get(strmap m, char *key);
void strmap_del(strmap *m, char *key);
void strmap_del_all(strmap *m, char *key);
void strmap_free(strmap m);
void strmap_free_all(strmap m);
void strmap_free_fn(strmap m, strmap_free_fn freefn);

#endif
