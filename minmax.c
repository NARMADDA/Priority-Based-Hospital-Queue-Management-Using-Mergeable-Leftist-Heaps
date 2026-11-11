/* minmax.c - min-max heap (Unit III) used as an array-based baseline.
   Even levels are "min" levels (best priority), odd levels are "max" levels
   (worst priority). Merging = append the other array and rebuild: O(n). */
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pq.h"

typedef struct { Patient *a; int n, cap; } MM;

#define LT(i, j) higher_priority(&h->a[i], &h->a[j])   /* a[i] before a[j] */

static int is_min_level(int i) {
    int lvl = 0;
    unsigned x = (unsigned)i + 1;
    while (x > 1) { x >>= 1; lvl++; }
    return (lvl & 1) == 0;
}
static void swp(MM *h, int i, int j) { Patient t = h->a[i]; h->a[i] = h->a[j]; h->a[j] = t; }

/* best (want_min) or worst child/grandchild of i */
static int pick(MM *h, int i, int want_min) {
    int c[6] = { 2*i+1, 2*i+2, 4*i+3, 4*i+4, 4*i+5, 4*i+6 };
    int best = -1;
    for (int k = 0; k < 6; k++) {
        if (c[k] >= h->n) continue;
        if (best < 0 || (want_min ? LT(c[k], best) : LT(best, c[k]))) best = c[k];
    }
    return best;
}

static void push_down(MM *h, int i) {
    int want_min = is_min_level(i);
    while (2*i + 1 < h->n) {
        int m = pick(h, i, want_min);
        int better = want_min ? LT(m, i) : LT(i, m);
        if (m >= 4*i + 3) {                         /* m is a grandchild */
            if (!better) break;
            swp(h, i, m);
            int p = (m - 1) / 2;
            if (want_min ? LT(p, m) : LT(m, p)) swp(h, m, p);
            i = m;
        } else {                                    /* m is a child */
            if (better) swp(h, i, m);
            break;
        }
    }
}

static void bubble_dir(MM *h, int i, int want_min) {
    while (i >= 3) {
        int gp = ((i - 1) / 2 - 1) / 2;
        if (want_min ? LT(i, gp) : LT(gp, i)) { swp(h, i, gp); i = gp; }
        else break;
    }
}
static void bubble_up(MM *h, int i) {
    if (i == 0) return;
    int p = (i - 1) / 2;
    if (is_min_level(i)) {
        if (LT(p, i)) { swp(h, i, p); bubble_dir(h, p, 0); }
        else bubble_dir(h, i, 1);
    } else {
        if (LT(i, p)) { swp(h, i, p); bubble_dir(h, p, 1); }
        else bubble_dir(h, i, 0);
    }
}

static void rebuild(MM *h) { for (int i = h->n / 2 - 1; i >= 0; i--) push_down(h, i); }

static void reserve(MM *h, int need) {
    if (need <= h->cap) return;
    int nc = h->cap ? h->cap : 16;
    while (nc < need) nc *= 2;
    Patient *na = realloc(h->a, (size_t)nc * sizeof(Patient));
    if (!na) { fprintf(stderr, "Fatal error: out of space\n"); exit(1); }
    h->a = na; h->cap = nc;
}

static void *m_create(void) { return calloc(1, sizeof(MM)); }
static void m_destroy(void *q) { MM *h = q; free(h->a); free(h); }
static void m_insert(void *q, const Patient *p) {
    MM *h = q;
    reserve(h, h->n + 1);
    h->a[h->n++] = *p;
    bubble_up(h, h->n - 1);
}
static int m_extract(void *q, Patient *out) {
    MM *h = q;
    if (h->n == 0) return 0;
    *out = h->a[0];
    h->a[0] = h->a[--h->n];
    if (h->n > 0) push_down(h, 0);
    return 1;
}
static int m_peek(void *q, Patient *out) {
    MM *h = q;
    if (h->n == 0) return 0;
    *out = h->a[0];
    return 1;
}
static int m_size(void *q) { return ((MM *)q)->n; }
static void m_merge_into(void *dst, void *src) {
    MM *d = dst, *s = src;
    reserve(d, d->n + s->n);
    if (s->n) memcpy(d->a + d->n, s->a, (size_t)s->n * sizeof(Patient));
    d->n += s->n;
    s->n = 0;
    rebuild(d);                                   /* O(n) */
}
static void m_bulk(void *q, const Patient *a, int n) {
    MM *h = q;
    reserve(h, h->n + n);
    if (n) memcpy(h->a + h->n, a, (size_t)n * sizeof(Patient));
    h->n += n;
    rebuild(h);
}

const PQOps MINMAX_OPS = { "Min-max heap (rebuild)", m_create, m_destroy, m_insert,
                           m_extract, m_peek, m_size, m_merge_into, m_bulk };
