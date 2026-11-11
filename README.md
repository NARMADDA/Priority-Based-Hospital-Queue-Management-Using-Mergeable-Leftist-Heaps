# Priority-Based Hospital Queue Management Using Mergeable Leftist Heaps

ADSA mini project (IF3151). Each hospital department keeps its own priority queue of
waiting patients. When a department closes, its whole queue is merged into another
department's queue. A **leftist heap** does this in O(log n); the baselines (min-max heap
and insertion-sorted array) need O(n).

## Build and run
    make
    ./hospital            # interactive menu
    ./hospital demo       # simulation summary
    ./hospital verbose    # every treatment printed
    ./hospital bench      # merge benchmark
    ./hospital test       # correctness self-test

No make (e.g. Windows with MinGW):
    gcc -O2 -o hospital main.c sim.c bench.c leftist.c minmax.c sortedarr.c

## Files
| File | Purpose |
|---|---|
| patient.h | Patient record and the one priority rule: severity, then arrival, then id |
| pq.h | Common interface so the simulation can run on any of the three structures |
| leftist.c | Leftist heap (Exp 11 adapted): merge, insert, delete-min |
| minmax.c | Min-max heap baseline; merge = append + rebuild |
| sortedarr.c | Insertion-sorted array baseline; merge = linear two-way merge |
| sim.c | Tick-based simulation: arrivals, doctors, department close/reopen events, statistics |
| bench.c | Merge benchmark and self-test |
| main.c | Menu |

## How the simulation works
Each tick: (1) scripted events run, (2) patients arrive and join their department's queue
(redirected if it is closed), (3) every free doctor takes the highest-priority patient of
their queue. Doctors finish their current patient before a closure takes effect for them.
Default scenario: 150 patients, 3 departments (Emergency x2 doctors, General x2,
Pediatrics x1), General closes at t=30, Pediatrics at t=45 (both into Emergency),
General reopens at t=60.

## What the output shows
- **Timeline:** queue sizes before and after each merge and who is seen next.
- **Waiting-time table by severity:** shows priority order working.
- **Same schedule on all structures:** identical start times and doctors for every patient,
  proof that the merge preserves priority order.
- **Benchmark Table 1:** merge time grows slowly for leftist, linearly for the baselines.
- **Benchmark Table 2:** honest trade-off; for plain insert/extract no structure clearly dominates.

## Possible extensions
Aging to prevent starvation, lazy-deletion "patient worsens" event, user-defined event script,
splitting a queue back on reopen (needs a different structure).
