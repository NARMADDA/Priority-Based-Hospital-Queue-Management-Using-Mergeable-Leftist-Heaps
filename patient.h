#ifndef PATIENT_H
#define PATIENT_H

typedef struct {
    int  id;
    int  severity;   
    int  arrival;    
    int  duration;   
    char name[16];
} Patient;

static inline int higher_priority(const Patient *a, const Patient *b) {
    if (a->severity != b->severity) return a->severity < b->severity;
    if (a->arrival  != b->arrival)  return a->arrival  < b->arrival;
    return a->id < b->id;
}
#endif
