#include <stdlib.h>
#include "map.h"

static inline struct strnode *new_strnode(uint64_t hash, char *key, void *val) {
	struct strnode *n = calloc(1, sizeof(struct strnode));
	n->hash = hash;
	n->key = key;
	n->val = val;

	return n;
}

static void _strmap_set(struct strnode **m, uint64_t hash, char *key, void *val) {
	if (*m == NULL) {
		*m = new_strnode(hash, key, val);
	} else if (hash == (*m)->hash) {
		(*m)->val = val;
	} else if (hash < (*m)->hash) {
		_strmap_set(&(*m)->l, hash, key, val);
	} else {
		_strmap_set(&(*m)->r, hash, key, val);
	}
}

static void *_strmap_get(struct strnode *m, uint64_t hash) {
	if (m == NULL) {
		return NULL;
	} else if (m->hash == hash) {
		return m->val;
	} else {
		return _strmap_get(hash < m->hash ? m->l : m->r, hash);
	}
}

static void _strmap_add_strnode(struct strnode **m, struct strnode *n) {
	if (*m == NULL) {
		*m = n;
	} else {
		_strmap_add_strnode(n->hash < (*m)->hash ? &(*m)->l : &(*m)-> r, n);
	}
}

static void _strmap_del_all(struct strnode **root, struct strnode **strnode, uint64_t hash) {
	struct strnode *n = *strnode;

	if (n != NULL) {
		if (hash == n->hash) {
			*strnode = NULL;
			free(n->key);
			free(n->val);
			if (n->l != NULL) _strmap_add_strnode(root, n->l);
			if (n->r != NULL) _strmap_add_strnode(root, n->r);
			free(n);
		} else {
			_strmap_del_all(root, hash < n->hash ? &n->l : &n->r, hash);
		}
	}
}

static void _strmap_del(struct strnode **root, struct strnode **strnode, uint64_t hash) {
	struct strnode *n = *strnode;

	if (n != NULL) {
		if (hash == n->hash) {
			*strnode = NULL;
			if (n->l != NULL) _strmap_add_strnode(root, n->l);
			if (n->r != NULL) _strmap_add_strnode(root, n->r);
			free(n);
		} else {
			_strmap_del(root, hash < n->hash ? &n->l : &n->r, hash);
		}
	}
}

// Taken from: https://github.com/haipome/fnv/blob/master/fnv.c#L368
static inline uint64_t fnv64a(char *key) {
	uint64_t hash = 0xcbf29ce484222325ULL;
	char *s = key;

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

void strmap_del_all(strmap *m, char *key) {
	_strmap_del_all(m, m, fnv64a(key));
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

void strmap_free_fn(strmap m, free_fn freefn) {
	if (m != NULL) {
		freefn(m->key, m->val);
		if (m->l != NULL) strmap_free_fn(m->l, freefn);
		if (m->r != NULL) strmap_free_fn(m->r, freefn);
		free(m);
	}
}

