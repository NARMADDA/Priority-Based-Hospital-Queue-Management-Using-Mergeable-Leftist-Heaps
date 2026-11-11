#ifndef HOSPITAL_H
#define HOSPITAL_H
#include "pq.h"

#define NDEPT 3
extern const char *DEPT_NAME[NDEPT];

enum { EV_CLOSE = 0, EV_REOPEN = 1 };
typedef struct { int tick, type, dept, target; } Event;

typedef struct {
    int num_patients, span;       
    unsigned seed;
    int n_events;
    Event events[8];              
} Scenario;

typedef struct {
    int n;
    int *start, *doc;            
    int cnt[6]; long sum[6]; int max[6];   
    int makespan;
    double merge_ns; int merges, moved;
} SimResult;

Scenario default_scenario(void);
void print_scenario(const Scenario *sc);
int  run_sim(const PQOps *ops, const Scenario *sc, SimResult *r, int level);
void print_report(const SimResult *r);
int  sim_same(const SimResult *a, const SimResult *b);
void sim_free(SimResult *r);

void run_bench(void);
int  run_selftest(void);
#endif
