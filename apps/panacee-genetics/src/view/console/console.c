/**
 * @file console.c
 * @brief Implementation of the console view (see console.h).
 */

#include "console.h"
#include "../../model/config.h"

#include <stdio.h>

void console_seed(const char *source, unsigned int seed)
{
    printf("Seed (%s): %u\n", source, seed);
}

void console_coverage_start(void)
{
    printf("Precomputing coverage map...\n");
}

void console_coverage_done(void)
{
    printf("Coverage map done.\n");
}

void console_dataset_mismatch(int data_total, int spec_total)
{
    fprintf(stderr, "Warning: dataset total (%d) differs from spec total (%d)\n", data_total, spec_total);
}

void console_generation(int gen, double best_score, int hospital_count, int uhc_count, int distant_count,
                        double distant_percent)
{
    printf("Gen %4d | fitness: %.0f | hop: %d | CHRU: %d | desert: %d (%.1f%%)\n", gen, best_score, hospital_count,
           uhc_count, distant_count, distant_percent);
}

void console_stagnation(int stagnation, double mutation_rate)
{
    printf("Stagnation for %d generations -> mutation_rate = %.2f\n", stagnation, mutation_rate);
}

void console_stagnation_stop(int stagnation_limit)
{
    printf("Stop: stagnation over %d generations.\n", stagnation_limit);
}

void console_local_search_start(void)
{
    printf("Running local search...\n");
}

void console_local_search_done(void)
{
    printf("Local search done.\n");
}

void console_final_result(int hospital_count, int total_beds, int covered_inhabitants, double fitness_score)
{
    printf("\n=== Final result: %d hospitals ===\n", hospital_count);
    printf("Total beds: %d\n", total_beds);
    printf("Covered inhabitants: %d\n", covered_inhabitants);
    printf("Fitness Score : %f\n", fitness_score);

    if (covered_inhabitants > 0)
    {
        printf("Beds per 1000 covered inhabitants: %.2f (target: %.2f)\n",
               (double)total_beds / (covered_inhabitants / 1000.0), BEDS_PER_INHABITANT);
    }
}
