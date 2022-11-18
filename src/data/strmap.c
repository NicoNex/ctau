#include <stdlib.h>
#include "map.h"

static inline struct node *new_node(uint64_t hash, char *key, void *val) {
	struct node *n = calloc(1, sizeof(struct node));
	n->hash = hash;
	n->key = key;
	n->val = val;

	return n;
}

static void _strmap_set(struct node **m, uint64_t hash, char *key, void *val) {
	

	if (*m == NULL) {
		*m = new_node(hash, key, val);
	} else if (hash == (*m)->hash) {
		(*m)->val = val;
	} else if (hash < (*m)->hash) {
		_strmap_set(&(*m)->l, hash, key, val);
	} else {
		_strmap_set(&(*m)->r, hash, key, val);
	}
}

static void *_strmap_get(struct node *m, uint64_t hash) {
	if (m == NULL) {
		return NULL;
	} else if (m->hash == hash) {
		return m->val;
	} else {
		return _strmap_get(hash < m->hash ? m->l : m->r, hash);
	}
}

static void _strmap_add_node(struct node **m, struct node *n) {
	if (*m == NULL) {
		*m = n;
	} else {
		_strmap_add_node(n->hash < (*m)->hash ? &(*m)->l : &(*m)-> r, n);
	}
}

static void _strmap_del(struct node **root, struct node **node, uint64_t hash) {
	struct node *n = *node;

	if (n != NULL) {
		if (hash == n->hash) {
			*node = NULL;
			free(n->key);
			free(n->val);
			if (n->l != NULL) _strmap_add_node(root, n->l);
			if (n->r != NULL) _strmap_add_node(root, n->r);
			free(n);
		} else {
			_strmap_del(root, hash < n->hash ? &n->l : &n->r, hash);
		}
	}
}

static void _strmap_del_stack(struct node **root, struct node **node, uint64_t hash) {
	struct node *n = *node;

	if (n != NULL) {
		if (hash == n->hash) {
			*node = NULL;
			if (n->l != NULL) _strmap_add_node(root, n->l);
			if (n->r != NULL) _strmap_add_node(root, n->r);
			free(n);
		} else {
			_strmap_del_stack(root, hash < n->hash ? &n->l : &n->r, hash);
		}
	}
}

// Taken from: https://github.com/haipome/fnv/blob/master/fnv.c#L368
static inline uint64_t fnv64a(char *key) {
	uint64_t hash = 0xcbf29ce484222325ULL;
	uint8_t *s = key;

	while (*s) {
		hash ^= (uint64_t)*s++;

		hash += (hash << 1) + (hash << 4) + (hash << 5) +
				(hash << 7) + (hash << 8) + (hash << 40);
	}

	return hash;
}

void strmap_set(strmap *m, char *key, void *val) {
	_strmap_set(m, fnv64a(key), key, val);
}

void *strmap_get(strmap m, char *key) {
	return _strmap_get(m, fnv64a(key));
}

void strmap_del(strmap *m, char *key) {
	_strmap_del(m, m, fnv64a(key));
}

void strmap_del_stack(strmap *m, char *key) {
	_strmap_del_stack(m, m, fnv64a(key));
}

void strmap_free_all(strmap m) {
	if (m != NULL) {
		free(m->key);
		free(m->val);
		if (m->l != NULL) strmap_free_all(m->l);
		if (m->r != NULL) strmap_free_all(m->r);
		free(m);
	}
}

void strmap_free(strmap m) {
	if (m != NULL) {
		if (m->l != NULL) strmap_free(m->l);
		if (m->r != NULL) strmap_free(m->r);
		free(m);
	}
}
