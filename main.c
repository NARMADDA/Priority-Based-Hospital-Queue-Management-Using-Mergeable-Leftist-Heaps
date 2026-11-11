/* main.c - menu for the hospital queue simulator. */
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hospital.h"

static void run_demo(const Scenario *sc, int verbose) {
    const PQOps *ops[3] = { &LEFTIST_OPS, &MINMAX_OPS, &SORTED_OPS };
    SimResult res[3];
    int ok[3];
    printf("\n========== HOSPITAL QUEUE SIMULATOR ==========\n");
    print_scenario(sc);
    printf("\n--- Timeline (leftist heap) ---\n");
    for (int i = 0; i < 3; i++) ok[i] = run_sim(ops[i], sc, &res[i], i == 0 ? (verbose ? 2 : 1) : 0);
    print_report(&res[0]);

    printf("\n--- Same scenario on all three queue structures ---\n");
    printf("%-26s %-8s %-10s %-18s %s\n", "Structure", "Merges", "Moved", "Merge time (us)", "Same schedule as leftist?");
    for (int i = 0; i < 3; i++)
        printf("%-26s %-8d %-10d %-18.2f %s\n", ops[i]->name, res[i].merges, res[i].moved,
               res[i].merge_ns / 1000.0, !ok[i] ? "ERROR" : (sim_same(&res[0], &res[i]) ? "YES" : "NO"));
    printf("\nQueues in this scenario are small, so the merge times above are tiny and noisy.\n"
           "Use the benchmark (option 3) to see how merge cost grows with queue size.\n");
    for (int i = 0; i < 3; i++) sim_free(&res[i]);
}

static int read_int(const char *prompt, int lo, int hi, int dflt) {
    int v;
    printf("%s [%d-%d, default %d]: ", prompt, lo, hi, dflt);
    if (scanf("%d", &v) != 1 || v < lo || v > hi) { int c; while ((c = getchar()) != '\n' && c != EOF) {} return dflt; }
    return v;
}

int main(int argc, char **argv) {
    Scenario sc = default_scenario();
    if (argc > 1) {                                  /* non-interactive: ./hospital demo|verbose|bench|test */
        if (!strcmp(argv[1], "demo"))    run_demo(&sc, 0);
        else if (!strcmp(argv[1], "verbose")) run_demo(&sc, 1);
        else if (!strcmp(argv[1], "bench"))   run_bench();
        else if (!strcmp(argv[1], "test"))    return run_selftest() ? 0 : 1;
        else { printf("usage: %s [demo|verbose|bench|test]\n", argv[0]); return 1; }
        return 0;
    }
    for (;;) {
        printf("\n---- HOSPITAL QUEUE SIMULATOR (leftist heaps) ----\n");
        printf("1. Run simulation\n2. Run simulation with every treatment printed\n"
               "3. Merge benchmark (leftist vs min-max vs sorted array)\n"
               "4. Correctness self-test\n5. Change scenario (patients, seed)\n6. Exit\n");
        int c = read_int("Enter your choice", 1, 6, 6);
        if (c == 1) run_demo(&sc, 0);
        else if (c == 2) run_demo(&sc, 1);
        else if (c == 3) run_bench();
        else if (c == 4) { printf("\nSelf-test:\n"); printf("%s\n", run_selftest() ? "ALL TESTS PASSED" : "SOME TESTS FAILED"); }
        else if (c == 5) {
            sc.num_patients = read_int("Number of patients", 10, 5000, sc.num_patients);
            sc.seed = (unsigned)read_int("Random seed", 1, 100000, (int)sc.seed);
        } else break;
    }
    return 0;
}
