#include "genetic.h"
#include "infrastructure/exporter/exporter.h"
#include "presentation/map/map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int *build_insee_to_idx(const Town *towns, int town_count)
{
    int *table = malloc(INSEE_MAX * sizeof(int));
    int i;

    for (i = 0; i < INSEE_MAX; i++)
        table[i] = -1;
    for (i = 0; i < town_count; i++)
    {
        if (towns[i].insee < 0 || towns[i].insee >= INSEE_MAX)
        {
            fprintf(stderr,
                    "Warning: INSEE %d out of [0,%d), skipping\n",
                    towns[i].insee, INSEE_MAX);
            continue;
        }
        table[towns[i].insee] = i;
    }
    return table;
}

static void compute_covered(char *covered,
                            const Individual *ind, int town_count,
                            const int *insee_to_idx,
                            int **coverage, const int *coverage_size)
{
    int i, k;
    memset(covered, 0, town_count);
    for (i = 0; i < ind->size; i++)
    {
        int j = insee_to_idx[ind->hospitals[i].insee];
        if (j < 0) continue;
        for (k = 0; k < coverage_size[j]; k++)
            covered[coverage[j][k]] = 1;
    }
}

static void local_search(Individual *result, const Town *towns, int town_count,
                         const int *insee_to_idx,
                         int **coverage, const int *coverage_size)
{
    int improved = 1;
    int i, k;

    while (improved)
    {
        int *cover_count = calloc(town_count, sizeof(int));
        int best = -1;
        int best_gain = 0;

        improved = 0;
        for (i = 0; i < result->size; i++)
        {
            int j = insee_to_idx[result->hospitals[i].insee];
            if (j >= 0)
                for (k = 0; k < coverage_size[j]; k++)
                    cover_count[coverage[j][k]]++;
        }

        for (i = 0; i < town_count; i++)
        {
            int gain;
            if (cover_count[i] > 0)
                continue;
            gain = 0;
            for (k = 0; k < coverage_size[i]; k++)
                if (cover_count[coverage[i][k]] == 0)
                    gain += towns[coverage[i][k]].inhabitants_count;
            if (gain > best_gain)
            {
                best_gain = gain;
                best = i;
            }
        }
        if (best >= 0 && best_gain > PENALTY_HOSPITAL)
        {
            result->hospitals[result->size].insee = towns[best].insee;
            result->hospitals[result->size].beds_count = 0;
            result->size++;
            improved = 1;
        }
        free(cover_count);
    }
}

static void init_seed(void)
{
    const char *env_seed = getenv("PANACEE_SEED");
    unsigned int seed;
    if (env_seed && *env_seed)
    {
        seed = (unsigned int)strtoul(env_seed, NULL, 10);
        printf("Seed (env): %u\n", seed);
    }
    else
    {
        seed = (unsigned int)time(NULL);
        printf("Seed (time): %u\n", seed);
    }
    srand(seed);
}

Individual run_genetic(const Town *towns, int town_count)
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
    int total_beds;
    int covered_inhabitants;
    char *covered_overlay;
    MapView view;

    init_seed();

    map_init(&view, towns, town_count);

    insee_to_idx = build_insee_to_idx(towns, town_count);

    map_draw_loading(&view, towns, town_count, "Calcul de la couverture...");
    precompute_coverage(towns, town_count, &coverage, &coverage_size);

    total_inhabitants = TOTAL_INHABITANTS;
    {
        int data_total = inhabitant_count(towns, town_count);
        if (data_total != TOTAL_INHABITANTS)
            fprintf(stderr,
                    "Warning: dataset total (%d) differs from spec total (%d)\n",
                    data_total, TOTAL_INHABITANTS);
    }

    map_draw_loading(&view, towns, town_count, "Initialisation de la population...");
    pop = init_population(towns, town_count, coverage, coverage_size);

    map_draw_loading(&view, towns, town_count, "Evaluation de la generation initiale...");
    evaluate_population(&pop, towns, town_count, insee_to_idx, coverage,
                        coverage_size, total_inhabitants);

    for (gen = 0; gen < MAX_GENERATIONS; gen++)
    {
        Individual current_best = best_individual(&pop);
        double best_score = current_best.fitness.fitness_score;
        int improved = best_score > prev_best;

        if (improved || gen % PROGRESS_INTERVAL == 0)
            printf("Gen %4d | fitness: %.0f | hop: %d | CHRU: %d | desert: %d (%.1f%%)\n",
                   gen,
                   best_score,
                   current_best.fitness.hospital_count,
                   current_best.fitness.uhc_count,
                   current_best.fitness.distant_resident_count,
                   (double)current_best.fitness.distant_resident_percent);

        if (improved)
        {
            prev_best = best_score;
            stagnation = 0;
            current_mutation_rate = MUTATION_RATE;
        }
        else
        {
            stagnation++;

            /* Progressively increase mutation rate to escape local optima */
            if (stagnation % 10 == 0 && current_mutation_rate < MUTATION_RATE_MAX)
            {
                double next = current_mutation_rate * 1.3;
                current_mutation_rate = next < MUTATION_RATE_MAX ? next : MUTATION_RATE_MAX;
                printf("Stagnation at gen %d -> mutation_rate = %.2f\n",
                       stagnation, current_mutation_rate);
            }
        }

        if (stagnation >= STAGNATION_LIMIT)
        {
            printf("Stop: stagnation over %d generations.\n", STAGNATION_LIMIT);
            break;
        }

        map_draw_state(&view, towns, town_count,
                       &current_best, insee_to_idx,
                       gen, stagnation, STAGNATION_LIMIT,
                       current_mutation_rate,
                       current_best.fitness.fitness_average,
                       stagnation == 0);

        /* Build next generation */
        next_pop.size = POPULATION_SIZE;
        next_pop.individuals = malloc(POPULATION_SIZE * sizeof(Individual));

        /* Elitism: carry the top ELITE_COUNT individuals unchanged */
        {
            char used[POPULATION_SIZE];
            int e, idx;
            memset(used, 0, sizeof(used));
            for (e = 0; e < ELITE_COUNT; e++)
            {
                int best_idx = -1;
                for (idx = 0; idx < POPULATION_SIZE; idx++)
                {
                    if (used[idx]) continue;
                    if (best_idx < 0 ||
                        pop.individuals[idx].fitness.fitness_score >
                        pop.individuals[best_idx].fitness.fitness_score)
                        best_idx = idx;
                }
                used[best_idx] = 1;

                next_pop.individuals[e].hospitals =
                    malloc(town_count * sizeof(Hospital));
                next_pop.individuals[e].size = pop.individuals[best_idx].size;
                memcpy(next_pop.individuals[e].hospitals,
                       pop.individuals[best_idx].hospitals,
                       pop.individuals[best_idx].size * sizeof(Hospital));
                next_pop.individuals[e].fitness =
                    pop.individuals[best_idx].fitness;
            }
        }

        for (i = ELITE_COUNT; i < POPULATION_SIZE; i++)
        {
            Individual p1 = tournament_select(&pop);
            Individual p2 = tournament_select(&pop);
            next_pop.individuals[i] = crossover(&p1, &p2, town_count);
            mutate(&next_pop.individuals[i], towns, town_count,
                   current_mutation_rate, coverage, coverage_size, insee_to_idx);
            remove_redundant(&next_pop.individuals[i], coverage, coverage_size,
                             insee_to_idx, town_count);
        }

        free_population(&pop);
        pop = next_pop;
        evaluate_population(&pop, towns, town_count, insee_to_idx,
                            coverage, coverage_size, total_inhabitants);
    }

    /* Copy the best solution before freeing the population */
    {
        Individual b = best_individual(&pop);
        result.hospitals = malloc(town_count * sizeof(Hospital));
        result.size = b.size;
        result.fitness = b.fitness;
        memcpy(result.hospitals, b.hospitals, b.size * sizeof(Hospital));
    }
    free_population(&pop);

    printf("Running local search...\n");
    local_search(&result, towns, town_count, insee_to_idx, coverage, coverage_size);
    printf("Local search done.\n");

    evaluate(&result, towns, town_count, insee_to_idx, coverage,
             coverage_size, total_inhabitants);

    compute_beds(&result, towns, town_count, insee_to_idx, coverage, coverage_size);

    printf("\n=== Final result: %d hospitals ===\n", result.size);

    total_beds = 0;
    covered_inhabitants = total_inhabitants - result.fitness.distant_resident_count;
    for (i = 0; i < result.size; i++)
        total_beds += result.hospitals[i].beds_count;
    printf("Total beds: %d\n", total_beds);
    printf("Covered inhabitants: %d\n", covered_inhabitants);
    printf("Fitness Score : %f\n", result.fitness.fitness_score);

    if (covered_inhabitants > 0)
        printf("Beds per 1000 covered inhabitants: %.2f (target: %.2f)\n",
               (double)total_beds / (covered_inhabitants / 1000.0),
               BEDS_PER_INHABITANT);

    export_result_csv(&result, towns, insee_to_idx, "./src/results/hospitals.csv");
    export_fitness_csv(&result.fitness, "./src/results/fitness.csv");
    export_towns_status_csv(&result, towns, town_count, insee_to_idx, coverage,
                            coverage_size, "./src/results/towns_status.csv");

    covered_overlay = calloc(town_count, 1);
    compute_covered(covered_overlay, &result, town_count,
                    insee_to_idx, coverage, coverage_size);
    map_draw_final(&view, towns, town_count, &result, insee_to_idx,
                   covered_overlay, total_beds);
    free(covered_overlay);

    map_close(&view);

    for (i = 0; i < town_count; i++)
        free(coverage[i]);
    free(coverage);
    free(coverage_size);
    free(insee_to_idx);

    return result;
}
