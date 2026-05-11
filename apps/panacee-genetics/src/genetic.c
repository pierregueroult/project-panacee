#include "genetic.h"
#include "infrastructure/exporter/exporter.h"
#include "presentation/map/map.h"

#include <limits.h>
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
                         int **coverage, const int *coverage_size,
                         int max_iter)
{
    int improved = 1;
    int i, k;
    int iter = 0;

    while (improved && iter < max_iter)
    {
        int *cover_count = calloc(town_count, sizeof(int));
        int best = -1;
        int best_gain = 0;

        improved = 0;
        iter++;
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
    int gen, i, isl;
    int stagnation = 0;
    int restarts = 0;
    double prev_best = -1;
    double current_mutation_rate = MUTATION_RATE;
    Individual result;
    Population islands[ISLAND_COUNT];
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

    map_draw_loading(&view, towns, town_count, "Initialisation des iles...");
    for (isl = 0; isl < ISLAND_COUNT; isl++)
        islands[isl] = init_population(towns, town_count,
                                       coverage, coverage_size, ISLAND_SIZE);

    map_draw_loading(&view, towns, town_count, "Evaluation de la generation initiale...");
    for (isl = 0; isl < ISLAND_COUNT; isl++)
        evaluate_population(&islands[isl], towns, town_count, insee_to_idx,
                            coverage, coverage_size, total_inhabitants);

    for (gen = 0; gen < MAX_GENERATIONS; gen++)
    {
        Individual current_best;
        double best_score;
        int improved;

        /* Global best across all islands */
        current_best = best_individual(&islands[0]);
        for (isl = 1; isl < ISLAND_COUNT; isl++)
        {
            Individual b = best_individual(&islands[isl]);
            if (b.fitness.fitness_score > current_best.fitness.fitness_score)
                current_best = b;
        }
        best_score = current_best.fitness.fitness_score;
        improved = best_score > prev_best;

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
            if (restarts >= MAX_RESTARTS)
            {
                printf("Stop: stagnation over %d generations after %d restarts.\n",
                       STAGNATION_LIMIT, MAX_RESTARTS);
                break;
            }

            /* Perturbed restart: keep the global elite, seed perturbed clones
               in island 0, and replace every island with a fresh init. */
            {
                Individual elite_copy;
                int best_isl = 0;
                int best_idx = 0;
                int c, m;

                for (isl = 0; isl < ISLAND_COUNT; isl++)
                    for (i = 0; i < islands[isl].size; i++)
                        if (islands[isl].individuals[i].fitness.fitness_score >
                            islands[best_isl].individuals[best_idx].fitness.fitness_score)
                        {
                            best_isl = isl;
                            best_idx = i;
                        }

                elite_copy.hospitals = malloc(town_count * sizeof(Hospital));
                elite_copy.size = islands[best_isl].individuals[best_idx].size;
                memcpy(elite_copy.hospitals,
                       islands[best_isl].individuals[best_idx].hospitals,
                       elite_copy.size * sizeof(Hospital));
                elite_copy.fitness =
                    islands[best_isl].individuals[best_idx].fitness;

                for (isl = 0; isl < ISLAND_COUNT; isl++)
                {
                    free_population(&islands[isl]);
                    islands[isl] = init_population(towns, town_count,
                                                   coverage, coverage_size,
                                                   ISLAND_SIZE);
                }

                /* Island 0 slot 0: untouched elite */
                free_individual(&islands[0].individuals[0]);
                islands[0].individuals[0] = elite_copy;

                /* Island 0 slots 1..PERTURB_CLONES: forced mutations on elite */
                for (c = 1; c <= PERTURB_CLONES && c < islands[0].size; c++)
                {
                    free_individual(&islands[0].individuals[c]);
                    islands[0].individuals[c].hospitals =
                        malloc(town_count * sizeof(Hospital));
                    islands[0].individuals[c].size = elite_copy.size;
                    memcpy(islands[0].individuals[c].hospitals,
                           elite_copy.hospitals,
                           elite_copy.size * sizeof(Hospital));
                    memset(&islands[0].individuals[c].fitness, 0,
                           sizeof(Fitness));
                    for (m = 0; m < PERTURB_MUTATIONS; m++)
                        mutate(&islands[0].individuals[c], towns, town_count,
                               1.0, coverage, coverage_size, insee_to_idx);
                }

                for (isl = 0; isl < ISLAND_COUNT; isl++)
                    evaluate_population(&islands[isl], towns, town_count,
                                        insee_to_idx, coverage, coverage_size,
                                        total_inhabitants);
            }

            restarts++;
            stagnation = 0;
            current_mutation_rate = MUTATION_RATE;
            printf("Restart %d/%d at gen %d\n", restarts, MAX_RESTARTS, gen);
            continue;
        }

        map_draw_state(&view, towns, town_count,
                       &current_best, insee_to_idx,
                       gen, stagnation, STAGNATION_LIMIT,
                       current_mutation_rate,
                       current_best.fitness.fitness_average,
                       stagnation == 0);

        /* Migration: ring topology, send MIGRATION_COUNT best from each
           island to the next, replacing that island's worst. */
        if (gen > 0 && gen % MIGRATION_INTERVAL == 0)
        {
            Individual migrants[ISLAND_COUNT][MIGRATION_COUNT];
            int m;

            for (isl = 0; isl < ISLAND_COUNT; isl++)
            {
                char used[ISLAND_SIZE];
                memset(used, 0, sizeof(used));
                for (m = 0; m < MIGRATION_COUNT; m++)
                {
                    int best_idx = -1;
                    int idx;
                    for (idx = 0; idx < ISLAND_SIZE; idx++)
                    {
                        if (used[idx]) continue;
                        if (best_idx < 0 ||
                            islands[isl].individuals[idx].fitness.fitness_score >
                            islands[isl].individuals[best_idx].fitness.fitness_score)
                            best_idx = idx;
                    }
                    used[best_idx] = 1;

                    migrants[isl][m].hospitals =
                        malloc(town_count * sizeof(Hospital));
                    migrants[isl][m].size =
                        islands[isl].individuals[best_idx].size;
                    memcpy(migrants[isl][m].hospitals,
                           islands[isl].individuals[best_idx].hospitals,
                           migrants[isl][m].size * sizeof(Hospital));
                    migrants[isl][m].fitness =
                        islands[isl].individuals[best_idx].fitness;
                }
            }

            for (isl = 0; isl < ISLAND_COUNT; isl++)
            {
                int dest = (isl + 1) % ISLAND_COUNT;
                char used[ISLAND_SIZE];
                memset(used, 0, sizeof(used));
                for (m = 0; m < MIGRATION_COUNT; m++)
                {
                    int worst_idx = -1;
                    int idx;
                    for (idx = 0; idx < ISLAND_SIZE; idx++)
                    {
                        if (used[idx]) continue;
                        if (worst_idx < 0 ||
                            islands[dest].individuals[idx].fitness.fitness_score <
                            islands[dest].individuals[worst_idx].fitness.fitness_score)
                            worst_idx = idx;
                    }
                    used[worst_idx] = 1;

                    free_individual(&islands[dest].individuals[worst_idx]);
                    islands[dest].individuals[worst_idx] = migrants[isl][m];
                }
            }
        }

        /* Evolve each island independently */
        {
            int effective_k = TOURNAMENT_K +
                (gen * (TOURNAMENT_K_MAX - TOURNAMENT_K)) / MAX_GENERATIONS;

            for (isl = 0; isl < ISLAND_COUNT; isl++)
            {
                Population *src = &islands[isl];
                Population next_island;
                char used[ISLAND_SIZE];
                int e, idx;

                next_island.size = ISLAND_SIZE;
                next_island.individuals =
                    malloc(ISLAND_SIZE * sizeof(Individual));

                /* Per-island elitism */
                memset(used, 0, sizeof(used));
                for (e = 0; e < ISLAND_ELITE; e++)
                {
                    int best_idx = -1;
                    for (idx = 0; idx < ISLAND_SIZE; idx++)
                    {
                        if (used[idx]) continue;
                        if (best_idx < 0 ||
                            src->individuals[idx].fitness.fitness_score >
                            src->individuals[best_idx].fitness.fitness_score)
                            best_idx = idx;
                    }
                    used[best_idx] = 1;

                    next_island.individuals[e].hospitals =
                        malloc(town_count * sizeof(Hospital));
                    next_island.individuals[e].size =
                        src->individuals[best_idx].size;
                    memcpy(next_island.individuals[e].hospitals,
                           src->individuals[best_idx].hospitals,
                           src->individuals[best_idx].size * sizeof(Hospital));
                    next_island.individuals[e].fitness =
                        src->individuals[best_idx].fitness;
                }

                for (i = ISLAND_ELITE; i < ISLAND_SIZE; i++)
                {
                    Individual p1 = tournament_select(src, effective_k);
                    Individual p2 = tournament_select(src, effective_k);
                    next_island.individuals[i] =
                        crossover(&p1, &p2, town_count);
                    mutate(&next_island.individuals[i], towns, town_count,
                           current_mutation_rate, coverage, coverage_size,
                           insee_to_idx);
                    remove_redundant(&next_island.individuals[i], coverage,
                                     coverage_size, insee_to_idx, town_count);
                    local_search(&next_island.individuals[i], towns, town_count,
                                 insee_to_idx, coverage, coverage_size,
                                 LOCAL_SEARCH_CHILD_ITER);
                }

                free_population(src);
                *src = next_island;
                evaluate_population(src, towns, town_count, insee_to_idx,
                                    coverage, coverage_size, total_inhabitants);
            }
        }
    }

    /* Copy the global best solution before freeing all islands */
    {
        int best_isl = 0;
        int best_idx = 0;

        for (isl = 0; isl < ISLAND_COUNT; isl++)
            for (i = 0; i < islands[isl].size; i++)
                if (islands[isl].individuals[i].fitness.fitness_score >
                    islands[best_isl].individuals[best_idx].fitness.fitness_score)
                {
                    best_isl = isl;
                    best_idx = i;
                }

        result.hospitals = malloc(town_count * sizeof(Hospital));
        result.size = islands[best_isl].individuals[best_idx].size;
        result.fitness = islands[best_isl].individuals[best_idx].fitness;
        memcpy(result.hospitals,
               islands[best_isl].individuals[best_idx].hospitals,
               result.size * sizeof(Hospital));
    }
    for (isl = 0; isl < ISLAND_COUNT; isl++)
        free_population(&islands[isl]);

    printf("Running local search...\n");
    local_search(&result, towns, town_count, insee_to_idx, coverage, coverage_size,
                 INT_MAX);
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
