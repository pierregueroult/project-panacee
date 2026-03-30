#include "genetic.h"
#include "infrastructure/exporter/exporter.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

Individual run_genetic(Town *towns, int town_count)
{
    int gen, i;
    int stagnation = 0;
    double prev_best = -1;
    double current_mutation_rate = MUTATION_RATE;
    Individual result;
    Population pop, next_pop;
    int *insee_to_idx;
    int **coverage;
    int *coverage_size;
    int total_inhabitants;

    /* seed RNG with current time for different results each run */
    srand((unsigned int)time(NULL));

    /* Build a lookup table insee -> town array index, built once for all evaluations */
    insee_to_idx = malloc(100000 * sizeof(int));
    for (i = 0; i < 100000; i++)
        insee_to_idx[i] = -1;
    for (i = 0; i < town_count; i++)
        insee_to_idx[towns[i].insee] = i;

    precompute_coverage(towns, town_count, &coverage, &coverage_size);

    total_inhabitants = inhabitant_count(towns, town_count);

    pop = init_population(towns, town_count, coverage, coverage_size);
    evaluate_population(&pop, towns, town_count, insee_to_idx, coverage, coverage_size, total_inhabitants);

    for (gen = 0; gen < MAX_GENERATIONS; gen++)
    {
        Individual current_best = best_individual(&pop);
        double best_score = current_best.fitness.fitness_score;

        printf("Gen %4d | fitness: %.0f | hop: %d | CHRU: %d | desert: %d (%.1f%%)\n",
               gen,
               best_score,
               current_best.fitness.hospital_count,
               current_best.fitness.uhc_count,
               current_best.fitness.distant_resident_count,
               (double)current_best.fitness.distant_resident_percent);

        if (best_score > prev_best)
        {
            prev_best = best_score;
            stagnation = 0;
            current_mutation_rate = MUTATION_RATE; /* reset mutation rate on improvement */
        }
        else
        {
            stagnation++;

            /* Progressively increase mutation rate to escape local optima */
            if (stagnation % 10 == 0 && current_mutation_rate < 0.4)
            {
                current_mutation_rate = current_mutation_rate * 1.3 < 0.4 ? current_mutation_rate * 1.3 : 0.4;
                printf("Stagnation at gen %d -> mutation_rate = %.2f\n",
                       stagnation, current_mutation_rate);
            }
        }

        if (stagnation >= STAGNATION_LIMIT)
        {
            printf("Stop: stagnation over %d generations.\n", STAGNATION_LIMIT);
            break;
        }

        /* Build next generation */
        next_pop.size = POPULATION_SIZE;
        next_pop.individuals = malloc(POPULATION_SIZE * sizeof(Individual));

        /* Elitism: carry the best individual unchanged into the next generation */

        Individual elite = best_individual(&pop);
        next_pop.individuals[0].hospitals = malloc(town_count * sizeof(Hospital));
        next_pop.individuals[0].size = elite.size;
        memcpy(next_pop.individuals[0].hospitals,
               elite.hospitals,
               elite.size * sizeof(Hospital));
        next_pop.individuals[0].fitness = elite.fitness;

        for (i = 1; i < POPULATION_SIZE; i++)
        {
            Individual p1 = tournament_select(&pop);
            Individual p2 = tournament_select(&pop);
            next_pop.individuals[i] = crossover(&p1, &p2, town_count);
            mutate(&next_pop.individuals[i], towns, town_count, current_mutation_rate,
                   coverage, coverage_size, insee_to_idx);
            remove_redundant(&next_pop.individuals[i], coverage, coverage_size,
                             insee_to_idx, town_count);
        }

        free_population(&pop);
        pop = next_pop;
        evaluate_population(&pop, towns, town_count, insee_to_idx, coverage, coverage_size, total_inhabitants);
    }

    /* Copy the best solution before freeing the population */

    Individual b = best_individual(&pop);
    result.hospitals = malloc(town_count * sizeof(Hospital)); /* town_count: upper bound for local search */
    result.size = b.size;
    result.fitness = b.fitness;
    memcpy(result.hospitals, b.hospitals, b.size * sizeof(Hospital));

    free_population(&pop);

    /* Local search: greedily add hospitals as long as the coverage gain > penalty */
    printf("Running local search...\n");
    {
        int improved = 1;
        int i, k;
        while (improved)
        {
            improved = 0;
            int *cover_count = calloc(town_count, sizeof(int));

            for (i = 0; i < result.size; i++)
            {
                int j = insee_to_idx[result.hospitals[i].insee];
                if (j >= 0)
                    for (k = 0; k < coverage_size[j]; k++)
                        cover_count[coverage[j][k]]++;
            }

            for (i = 0; i < town_count; i++)
            {
                if (cover_count[i] > 0)
                    continue;
                /* Count inhabitants newly covered by a hospital at town i */
                int gain = 0;
                for (k = 0; k < coverage_size[i]; k++)
                    if (cover_count[coverage[i][k]] == 0)
                        gain += towns[coverage[i][k]].inhabitants_count;
                if (gain > PENALITY_HOSPITAL)
                {
                    result.hospitals[result.size].insee = towns[i].insee;
                    result.hospitals[result.size].beds_count = 0;
                    result.size++;
                    for (k = 0; k < coverage_size[i]; k++)
                        cover_count[coverage[i][k]]++;
                    improved = 1;
                }
            }
            free(cover_count);
        }
    }
    printf("Local search done.\n");

    /* Compute beds per hospital (post-optimization) */
    compute_beds(&result, towns, town_count, insee_to_idx, coverage, coverage_size);

    printf("\n=== Final result: %d hospitals ===\n", result.size);

    int total_beds = 0;
    int covered_inhabitants = total_inhabitants - result.fitness.distant_resident_count;
    for (i = 0; i < result.size; i++)
        total_beds += result.hospitals[i].beds_count;
    printf("Total beds: %d\n", total_beds);
    printf("Covered inhabitants: %d\n", covered_inhabitants);
    printf("Fitness Score : %f\n", result.fitness.fitness_score);

    if (covered_inhabitants > 0)
        printf("Beds per 1000 covered inhabitants: %.2f (target: %.2f)\n",
               (double)total_beds / (covered_inhabitants / 1000.0), BEDS_PER_INHABITANT);

    export_result_csv(&result, towns, insee_to_idx, "./src/results/hospitals.csv");
    export_fitness_csv(&result.fitness, "./src/results/fitness.csv");

    for (i = 0; i < town_count; i++)
        free(coverage[i]);
    free(coverage);
    free(coverage_size);
    free(insee_to_idx);
    return result;
}
