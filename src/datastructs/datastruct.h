#ifndef DATASTRUCT_H_
#define DATASTRUCT_H_

#include <stdint.h>

struct keyhash {

};

typedef struct map {
	uint64_t key;
	void *val;
	struct map *l;
	struct map *r;
} map;

typedef struct slice {
	void *data;
	size_t len;
} slice;

#endif
