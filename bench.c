/* bench.c - merge benchmark and correctness self-test. */
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hospital.h"

static const PQOps *ALL[3] = { &LEFTIST_OPS, &MINMAX_OPS, &SORTED_OPS };

static Patient *make_patients(int n, int id0, Rng *rng) {
    Patient *a = calloc((size_t)n, sizeof(Patient));
    for (int i = 0; i < n; i++) {
        a[i].id = id0 + i;
        a[i].severity = 1 + rng_range(rng, 5);
        a[i].arrival  = rng_range(rng, 1000000);
        a[i].duration = 1 + rng_range(rng, 10);
    }
    return a;
}
static int cmp_d(const void *x, const void *y) {
    double a = *(const double *)x, b = *(const double *)y;
    return (a > b) - (a < b);
}

void run_bench(void) {
    const int merge_sizes[] = { 1000, 10000, 100000, 1000000 };
    const int RUNS = 3;
    Rng rng = { 7 };

    printf("\nTABLE 1: time to merge two queues of n patients each (median of %d runs, microseconds)\n", RUNS);
    printf("%-10s %-16s %-24s %-26s %s\n", "n", "Leftist merge", "Min-max rebuild", "Sorted-array merge", "Leftist speed-up vs min-max");
    for (int si = 0; si < 4; si++) {
        int n = merge_sizes[si];
        double med[3];
        for (int b = 0; b < 3; b++) {
            double t[RUNS];
            for (int r = 0; r < RUNS; r++) {
                Rng g = { (uint32_t)(n + r) };               /* same data for all back ends */
                Patient *A = make_patients(n, 1, &g), *B = make_patients(n, n + 1, &g);
                void *qa = ALL[b]->create(), *qb = ALL[b]->create();
                ALL[b]->bulk_load(qa, A, n);
                ALL[b]->bulk_load(qb, B, n);
                double t0 = now_ns();
                ALL[b]->merge_into(qa, qb);
                double t1 = now_ns();
                if (ALL[b]->size(qa) != 2 * n || ALL[b]->size(qb) != 0) { printf("size error\n"); exit(1); }
                t[r] = (t1 - t0) / 1000.0;
                ALL[b]->destroy(qa); ALL[b]->destroy(qb);
                free(A); free(B);
            }
            qsort(t, RUNS, sizeof(double), cmp_d);
            med[b] = t[RUNS / 2];
        }
        printf("%-10d %-16.2f %-24.2f %-26.2f %.0fx\n", n, med[0], med[1], med[2], med[1] / (med[0] > 0.001 ? med[0] : 0.001));
    }

    const int ie_sizes[] = { 1000, 10000, 50000 };
    printf("\nTABLE 2: the trade-off - time to insert n patients one by one, then extract all (milliseconds)\n");
    printf("%-10s %-16s %-24s %s\n", "n", "Leftist heap", "Min-max heap", "Sorted array");
    for (int si = 0; si < 3; si++) {
        int n = ie_sizes[si];
        Patient *A = make_patients(n, 1, &rng);
        double ms[3];
        for (int b = 0; b < 3; b++) {
            void *q = ALL[b]->create();
            Patient out;
            double t0 = now_ns();
            for (int i = 0; i < n; i++) ALL[b]->insert(q, &A[i]);
            while (ALL[b]->extract(q, &out)) {}
            ms[b] = (now_ns() - t0) / 1e6;
            ALL[b]->destroy(q);
        }
        printf("%-10d %-16.2f %-24.2f %.2f\n", n, ms[0], ms[1], ms[2]);
        free(A);
    }
    printf("\nReading the tables: merging is where the leftist heap wins (O(log n) vs O(n)).\n"
           "For plain insert/extract the result is mixed: leftist and min-max heap are both O(log n)\n"
           "per operation and neither dominates (leftist pays a malloc per node; the array heap has\n"
           "better cache locality), while the sorted array degrades towards O(n^2) overall.\n");
}

/* Insert two halves, merge, extract everything, check order and agreement. */
int run_selftest(void) {
    const int N = 20000;
    int ok = 1;
    Rng rng = { 99 };
    Patient *A = make_patients(N / 2, 1, &rng), *B = make_patients(N / 2, N / 2 + 1, &rng);
    int *seq[3];
    for (int b = 0; b < 3; b++) {
        const PQOps *o = ALL[b];
        void *qa = o->create(), *qb = o->create(), *qe = o->create();
        for (int i = 0; i < N / 2; i++) { o->insert(qa, &A[i]); o->insert(qb, &B[i]); }
        o->merge_into(qa, qe);                       /* merging an empty queue is a no-op */
        o->merge_into(qe, qa);                       /* into an empty queue moves everything */
        o->merge_into(qa, qe);
        o->merge_into(qa, qb);
        seq[b] = malloc((size_t)N * sizeof(int));
        Patient p, prev; int cnt = 0, ordered = 1;
        while (o->extract(qa, &p)) {
            if (cnt > 0 && higher_priority(&p, &prev)) ordered = 0;
            seq[b][cnt++] = p.id;
            prev = p;
        }
        int good = ordered && cnt == N && o->size(qb) == 0 && o->size(qe) == 0;
        printf("  %-26s extracted %d patients, priority order %s  -> %s\n", o->name, cnt,
               ordered ? "preserved" : "BROKEN", good ? "PASS" : "FAIL");
        ok &= good;
        o->destroy(qa); o->destroy(qb); o->destroy(qe);
    }
    int same = memcmp(seq[0], seq[1], N * sizeof(int)) == 0 && memcmp(seq[0], seq[2], N * sizeof(int)) == 0;
    printf("  All three structures give the identical service order -> %s\n", same ? "PASS" : "FAIL");
    ok &= same;
    for (int b = 0; b < 3; b++) free(seq[b]);
    free(A); free(B);
    return ok;
}
