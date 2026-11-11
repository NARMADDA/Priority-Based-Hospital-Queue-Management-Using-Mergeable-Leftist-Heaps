#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hospital.h"

const char *DEPT_NAME[NDEPT] = { "Emergency", "General", "Pediatrics" };
static const int DOCS_PER_DEPT[NDEPT] = { 2, 2, 1 };
#define TOTAL_DOCS 5

Scenario default_scenario(void) {
    Scenario s;
    memset(&s, 0, sizeof s);
    s.num_patients = 150;
    s.span = 80;
    s.seed = 42;
    s.events[0] = (Event){ 30, EV_CLOSE,  1, 0 };   
    s.events[1] = (Event){ 45, EV_CLOSE,  2, 0 };   
    s.events[2] = (Event){ 60, EV_REOPEN, 1, 0 };   
    s.n_events = 3;
    return s;
}

void print_scenario(const Scenario *sc) {
    printf("Scenario: %d patients arriving over ticks 0-%d, seed %u\n",
           sc->num_patients, sc->span - 1, sc->seed);
    printf("Doctors : ");
    for (int d = 0; d < NDEPT; d++)
        printf("%s x%d%s", DEPT_NAME[d], DOCS_PER_DEPT[d], d < NDEPT - 1 ? ", " : "\n");
    for (int i = 0; i < sc->n_events; i++) {
        const Event *e = &sc->events[i];
        if (e->type == EV_CLOSE)
            printf("Event   : t=%d  %s closes, its queue moves to %s\n",
                   e->tick, DEPT_NAME[e->dept], DEPT_NAME[e->target]);
        else
            printf("Event   : t=%d  %s reopens\n", e->tick, DEPT_NAME[e->dept]);
    }
}

typedef struct { Patient p; int dept, seq; } GenP;

static int cmp_gen(const void *x, const void *y) {
    const GenP *a = x, *b = y;
    if (a->p.arrival != b->p.arrival) return a->p.arrival - b->p.arrival;
    return a->seq - b->seq;
}

static int gen_patients(const Scenario *sc, Patient **pts, int **dept) {
    int n = sc->num_patients;
    Rng rng = { sc->seed * 2654435761u + 12345u };
    GenP *g = malloc((size_t)n * sizeof(GenP));
    for (int i = 0; i < n; i++) {
        int r = rng_range(&rng, 100);
        int d = r < 25 ? 0 : (r < 75 ? 1 : 2);
        int sev = d == 0 ? 1 + rng_range(&rng, 3)        
                : d == 1 ? 3 + rng_range(&rng, 3)        
                :          2 + rng_range(&rng, 4);      
        memset(&g[i], 0, sizeof g[i]);
        g[i].p.severity = sev;
        g[i].p.arrival  = rng_range(&rng, sc->span);
        g[i].p.duration = 2 + rng_range(&rng, 3) + (sev <= 2 ? 2 : 0);
        g[i].dept = d;
        g[i].seq = i;
    }
    qsort(g, (size_t)n, sizeof(GenP), cmp_gen);
    *pts = malloc((size_t)n * sizeof(Patient));
    *dept = malloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) {
        g[i].p.id = i + 1;
        snprintf(g[i].p.name, sizeof g[i].p.name, "P%03d", i + 1);
        (*pts)[i] = g[i].p;
        (*dept)[i] = g[i].dept;
    }
    free(g);
    return n;
}

int run_sim(const PQOps *ops, const Scenario *sc, SimResult *r, int level) {
    memset(r, 0, sizeof *r);
    Patient *pts; int *pdept;
    int n = gen_patients(sc, &pts, &pdept);
    r->n = n;
    r->start = malloc((size_t)n * sizeof(int));
    r->doc   = malloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) r->start[i] = r->doc[i] = -1;
    for (int s = 0; s < 6; s++) r->max[s] = 0;

    void *q[NDEPT];
    int open[NDEPT], route[NDEPT];
    for (int d = 0; d < NDEPT; d++) { q[d] = ops->create(); open[d] = 1; route[d] = d; }

    int doc_dept[TOTAL_DOCS], busy[TOTAL_DOCS], k = 0;
    for (int d = 0; d < NDEPT; d++)
        for (int j = 0; j < DOCS_PER_DEPT[d]; j++) { doc_dept[k] = d; busy[k] = 0; k++; }

    int next = 0, ev = 0, treated = 0;
    for (int t = 0; t < 1000000; t++) {
        /* 1. scripted events */
        while (ev < sc->n_events && sc->events[ev].tick <= t) {
            const Event *e = &sc->events[ev++];
            int d = e->dept;
            if (e->type == EV_REOPEN) {
                if (!open[d]) {
                    open[d] = 1; route[d] = d;
                    if (level >= 1) printf("[t=%3d] %s REOPENED\n", t, DEPT_NAME[d]);
                }
                continue;
            }
            if (!open[d]) continue;
            int dst = e->target;
            while (!open[dst]) dst = route[dst];
            if (dst == d) { if (level >= 1) printf("[t=%3d] cannot close %s: no other open department\n", t, DEPT_NAME[d]); continue; }
            int moved = ops->size(q[d]), before = ops->size(q[dst]);
            double t0 = now_ns();
            ops->merge_into(q[dst], q[d]);
            double t1 = now_ns();
            open[d] = 0; route[d] = dst;
            r->merge_ns += t1 - t0; r->merges++; r->moved += moved;
            if (level >= 1) {
                Patient nx;
                printf("[t=%3d] %s CLOSED: %d waiting patients merged into %s (queue %d -> %d)",
                       t, DEPT_NAME[d], moved, DEPT_NAME[dst], before, ops->size(q[dst]));
                if (ops->peek(q[dst], &nx))
                    printf("; next to be seen: %s (severity %d)", nx.name, nx.severity);
                printf("\n");
            }
        }
        while (next < n && pts[next].arrival <= t) {
            int d = pdept[next];
            while (!open[d]) d = route[d];
            ops->insert(q[d], &pts[next]);
            next++;
        }
        for (int j = 0; j < TOTAL_DOCS; j++) {
            int d = doc_dept[j];
            if (!open[d] || busy[j] > t) continue;
            Patient p;
            if (!ops->extract(q[d], &p)) continue;
            int wait = t - p.arrival;
            r->start[p.id - 1] = t; r->doc[p.id - 1] = j;
            busy[j] = t + p.duration;
            r->cnt[p.severity]++; r->sum[p.severity] += wait;
            if (wait > r->max[p.severity]) r->max[p.severity] = wait;
            treated++;
            if (level >= 2)
                printf("[t=%3d] %s Dr.%d starts %s (severity %d, waited %d)\n",
                       t, DEPT_NAME[d], j + 1, p.name, p.severity, wait);
        }
        int pending = next < n;
        for (int d = 0; d < NDEPT; d++) if (ops->size(q[d]) > 0) pending = 1;
        if (!pending) break;
    }
    for (int j = 0; j < TOTAL_DOCS; j++) if (busy[j] > r->makespan) r->makespan = busy[j];
    for (int d = 0; d < NDEPT; d++) ops->destroy(q[d]);
    free(pts); free(pdept);
    return treated == n;
}

void print_report(const SimResult *r) {
    printf("\nWaiting time by severity (ticks from arrival to start of treatment)\n");
    printf("  Severity  Treated  AvgWait  MaxWait\n");
    int tc = 0; long ts = 0; int tm = 0;
    for (int s = 1; s <= 5; s++) {
        if (!r->cnt[s]) continue;
        printf("  %5d     %5d   %6.2f   %6d\n", s, r->cnt[s], (double)r->sum[s] / r->cnt[s], r->max[s]);
        tc += r->cnt[s]; ts += r->sum[s]; if (r->max[s] > tm) tm = r->max[s];
    }
    printf("  %-8s  %5d   %6.2f   %6d\n", "All", tc, tc ? (double)ts / tc : 0.0, tm);
    printf("All treatment finished at tick %d.\n", r->makespan);
}

int sim_same(const SimResult *a, const SimResult *b) {
    return a->n == b->n &&
           memcmp(a->start, b->start, (size_t)a->n * sizeof(int)) == 0 &&
           memcmp(a->doc,   b->doc,   (size_t)a->n * sizeof(int)) == 0;
}
void sim_free(SimResult *r) { free(r->start); free(r->doc); r->start = r->doc = NULL; }
