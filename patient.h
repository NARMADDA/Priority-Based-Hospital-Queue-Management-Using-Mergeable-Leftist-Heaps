/* patient.h - patient record and the single priority rule used everywhere. */
#ifndef PATIENT_H
#define PATIENT_H

typedef struct {
    int  id;
    int  severity;   /* 1 = critical ... 5 = minor */
    int  arrival;    /* tick of arrival            */
    int  duration;   /* ticks of treatment needed  */
    char name[16];
} Patient;

/* 1 if a must be served before b.
   Order: severity, then arrival time, then id. The id makes the order total,
   so all data structures must produce exactly the same service order. */
static inline int higher_priority(const Patient *a, const Patient *b) {
    if (a->severity != b->severity) return a->severity < b->severity;
    if (a->arrival  != b->arrival)  return a->arrival  < b->arrival;
    return a->id < b->id;
}
#endif
