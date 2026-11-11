/* pq.h - one interface, three interchangeable priority-queue back ends. */
#ifndef PQ_H
#define PQ_H
#include "patient.h"

typedef struct {
    const char *name;
    void *(*create)(void);
    void  (*destroy)(void *q);
    void  (*insert)(void *q, const Patient *p);
    int   (*extract)(void *q, Patient *out);          /* 1 = ok, 0 = empty */
    int   (*peek)(void *q, Patient *out);             /* 1 = ok, 0 = empty */
    int   (*size)(void *q);
    void  (*merge_into)(void *dst, void *src);        /* src is left empty */
    void  (*bulk_load)(void *q, const Patient *a, int n);
} PQOps;

extern const PQOps LEFTIST_OPS;   /* proposed: native O(log n) merge          */
extern const PQOps MINMAX_OPS;    /* baseline: concatenate + rebuild, O(n)    */
extern const PQOps SORTED_OPS;    /* baseline: insertion-sorted array, O(n)   */
#endif
