/* util.h - timer and deterministic random numbers.
   Keep this as the FIRST include in every .c file. */
#ifndef UTIL_H
#define UTIL_H
#define _POSIX_C_SOURCE 199309L
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
static inline double now_ns(void) {
    LARGE_INTEGER f, c;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1e9 / (double)f.QuadPart;
}
#else
#include <time.h>
static inline double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}
#endif

/* Small LCG so every machine produces the same patients for the same seed. */
typedef struct { uint32_t s; } Rng;
static inline uint32_t rng_next(Rng *r) {
    r->s = r->s * 1664525u + 1013904223u;
    return r->s >> 8;
}
static inline int rng_range(Rng *r, int n) { return (int)(rng_next(r) % (uint32_t)n); }
#endif
