/* sortedarr.c - array kept sorted by insertion sort (Unit I) as a baseline.
   Best patient is stored at the END so extraction is O(1).
   Insert = one insertion-sort step, O(n). Merge = linear two-way merge, O(n). */
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pq.h"

typedef struct { Patient *a; int n, cap; } SA;

static void reserve(SA *h, int need) {
    if (need <= h->cap) return;
    int nc = h->cap ? h->cap : 16;
    while (nc < need) nc *= 2;
    Patient *na = realloc(h->a, (size_t)nc * sizeof(Patient));
    if (!na) { fprintf(stderr, "Fatal error: out of space\n"); exit(1); }
    h->a = na; h->cap = nc;
}

static void *s_create(void) { return calloc(1, sizeof(SA)); }
static void s_destroy(void *q) { SA *h = q; free(h->a); free(h); }

static void s_insert(void *q, const Patient *p) {
    SA *h = q;
    reserve(h, h->n + 1);
    int pos = h->n;
    while (pos > 0 && higher_priority(&h->a[pos - 1], p)) {   /* shift better ones right */
        h->a[pos] = h->a[pos - 1];
        pos--;
    }
    h->a[pos] = *p;
    h->n++;
}
static int s_extract(void *q, Patient *out) {
    SA *h = q;
    if (h->n == 0) return 0;
    *out = h->a[--h->n];
    return 1;
}
static int s_peek(void *q, Patient *out) {
    SA *h = q;
    if (h->n == 0) return 0;
    *out = h->a[h->n - 1];
    return 1;
}
static int s_size(void *q) { return ((SA *)q)->n; }

static void s_merge_into(void *dst, void *src) {
    SA *d = dst, *s = src;
    int total = d->n + s->n;
    if (s->n == 0) return;
    Patient *tmp = malloc((size_t)total * sizeof(Patient));
    if (!tmp) { fprintf(stderr, "Fatal error: out of space\n"); exit(1); }
    int i = 0, j = 0, k = 0;
    while (i < d->n && j < s->n) {              /* worst first, so best ends up last */
        if (higher_priority(&d->a[i], &s->a[j])) tmp[k++] = s->a[j++];
        else                                     tmp[k++] = d->a[i++];
    }
    while (i < d->n) tmp[k++] = d->a[i++];
    while (j < s->n) tmp[k++] = s->a[j++];
    free(d->a);
    d->a = tmp; d->n = total; d->cap = total;
    s->n = 0;
}

static int cmp_goodness(const void *x, const void *y) {
    const Patient *a = x, *b = y;
    if (higher_priority(a, b)) return 1;
    if (higher_priority(b, a)) return -1;
    return 0;
}
static void s_bulk(void *q, const Patient *a, int n) {
    SA *h = q;
    reserve(h, h->n + n);
    if (n) memcpy(h->a + h->n, a, (size_t)n * sizeof(Patient));
    h->n += n;
    qsort(h->a, (size_t)h->n, sizeof(Patient), cmp_goodness);
}

const PQOps SORTED_OPS = { "Sorted array (insertion)", s_create, s_destroy, s_insert,
                           s_extract, s_peek, s_size, s_merge_into, s_bulk };
