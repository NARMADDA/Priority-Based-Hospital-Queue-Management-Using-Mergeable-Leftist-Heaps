/* leftist.c - leftist heap (Unit III). Same idea as Exp 11, with the Patient
   record as the element and a cleaner merge. npl(NULL) = -1. */
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include "pq.h"

typedef struct LNode {
    Patient p;
    struct LNode *left, *right;
    int npl;                       /* null path length */
} LNode;

typedef struct { LNode *root; int size; } Leftist;

static int npl_of(const LNode *n) { return n ? n->npl : -1; }

/* Recursion only follows right paths, which have O(log n) length. */
static LNode *lmerge(LNode *a, LNode *b) {
    if (!a) return b;
    if (!b) return a;
    if (higher_priority(&b->p, &a->p)) { LNode *t = a; a = b; b = t; }
    a->right = lmerge(a->right, b);
    if (npl_of(a->left) < npl_of(a->right)) {      /* keep the heap leftist */
        LNode *t = a->left; a->left = a->right; a->right = t;
    }
    a->npl = npl_of(a->right) + 1;
    return a;
}

static void *l_create(void) { return calloc(1, sizeof(Leftist)); }

/* Left paths can be long, so free by rotation instead of recursion. */
static void l_clear(Leftist *h) {
    LNode *r = h->root;
    while (r) {
        if (r->left) { LNode *t = r->left; r->left = t->right; t->right = r; r = t; }
        else         { LNode *nx = r->right; free(r); r = nx; }
    }
    h->root = NULL; h->size = 0;
}
static void l_destroy(void *q) { l_clear(q); free(q); }

static void l_insert(void *q, const Patient *p) {
    Leftist *h = q;
    LNode *n = malloc(sizeof *n);
    if (!n) { fprintf(stderr, "Fatal error: out of space\n"); exit(1); }
    n->p = *p; n->left = n->right = NULL; n->npl = 0;
    h->root = lmerge(h->root, n);
    h->size++;
}
static int l_extract(void *q, Patient *out) {
    Leftist *h = q;
    if (!h->root) return 0;
    LNode *r = h->root;
    *out = r->p;
    h->root = lmerge(r->left, r->right);
    free(r);
    h->size--;
    return 1;
}
static int l_peek(void *q, Patient *out) {
    Leftist *h = q;
    if (!h->root) return 0;
    *out = h->root->p;
    return 1;
}
static int l_size(void *q) { return ((Leftist *)q)->size; }

/* THE operation this project is about: O(log n) queue consolidation. */
static void l_merge_into(void *dst, void *src) {
    Leftist *d = dst, *s = src;
    d->root = lmerge(d->root, s->root);
    d->size += s->size;
    s->root = NULL; s->size = 0;
}
static void l_bulk(void *q, const Patient *a, int n) {
    for (int i = 0; i < n; i++) l_insert(q, &a[i]);
}

const PQOps LEFTIST_OPS = { "Leftist heap", l_create, l_destroy, l_insert,
                            l_extract, l_peek, l_size, l_merge_into, l_bulk };
