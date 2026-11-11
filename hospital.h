#ifndef HOSPITAL_H
#define HOSPITAL_H
#include "pq.h"

#define NDEPT 3
extern const char *DEPT_NAME[NDEPT];

enum { EV_CLOSE = 0, EV_REOPEN = 1 };
typedef struct { int tick, type, dept, target; } Event;

typedef struct {
    int num_patients, span;       /* patients arrive at random ticks in [0, span) */
    unsigned seed;
    int n_events;
    Event events[8];              /* must be sorted by tick */
} Scenario;

typedef struct {
    int n;
    int *start, *doc;             /* per patient: start tick and doctor index */
    int cnt[6]; long sum[6]; int max[6];   /* waiting-time stats by severity */
    int makespan;
    double merge_ns; int merges, moved;
} SimResult;

Scenario default_scenario(void);
void print_scenario(const Scenario *sc);
/* level: 0 = silent, 1 = print queue-merge events, 2 = also print every treatment */
int  run_sim(const PQOps *ops, const Scenario *sc, SimResult *r, int level);
void print_report(const SimResult *r);
int  sim_same(const SimResult *a, const SimResult *b);
void sim_free(SimResult *r);

void run_bench(void);
int  run_selftest(void);
#endif
