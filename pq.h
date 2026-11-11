#ifndef PQ_H
#define PQ_H
#include "patient.h"

typedef struct {
    const char *name;
    void *(*create)(void);
    void  (*destroy)(void *q);
    void  (*insert)(void *q, const Patient *p);
    int   (*extract)(void *q, Patient *out);          
    int   (*peek)(void *q, Patient *out);             
    int   (*size)(void *q);
    void  (*merge_into)(void *dst, void *src);        
    void  (*bulk_load)(void *q, const Patient *a, int n);
} PQOps;

extern const PQOps LEFTIST_OPS;   
extern const PQOps MINMAX_OPS;    
extern const PQOps SORTED_OPS;   
#endif
