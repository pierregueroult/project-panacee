#include "genetic_controller.h"

#include "../model/config.h"
#include "../model/genetic/genetic.h"
#include "../model/individual/individual.h"
#include "../model/population/population.h"
#include "../model/town/town.h"
#include "../model/io/exporter/exporter.h"

#include "../view/console/console.h"
#include "../view/map/map.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

static void init_seed(void)
{
    const char *env_seed = getenv("PANACEE_SEED");
    unsigned int seed;
    if (env_seed && *env_seed)
    {
        seed = (unsigned int)strtoul(env_seed, NULL, 10);
        console_seed("env", seed);
    }
    else
    {
        seed = (unsigned int)time(NULL);
        console_seed("time", seed);
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
    console_coverage_start();
    precompute_coverage(towns, town_count, &coverage, &coverage_size);
    console_coverage_done();

    total_inhabitants = TOTAL_INHABITANTS;
    {
        int data_total = inhabitant_count(towns, town_count);
        if (data_total != TOTAL_INHABITANTS)
            console_dataset_mismatch(data_total, TOTAL_INHABITANTS);
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
            console_generation(gen, best_score,
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
            if (stagnation % 10 == 0 && current_mutation_rate < 0.4)
            {
                current_mutation_rate = current_mutation_rate * 1.3 < 0.4 ? current_mutation_rate * 1.3 : 0.4;
                console_stagnation(stagnation, current_mutation_rate);
            }
        }

        if (stagnation >= STAGNATION_LIMIT)
        {
            console_stagnation_stop(STAGNATION_LIMIT);
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

        /* Elitism: carry the best individual unchanged into the next generation */
        {
            Individual elite = best_individual(&pop);
            next_pop.individuals[0].hospitals =
                malloc(town_count * sizeof(Hospital));
            next_pop.individuals[0].size = elite.size;
            memcpy(next_pop.individuals[0].hospitals,
                   elite.hospitals,
                   elite.size * sizeof(Hospital));
            next_pop.individuals[0].fitness = elite.fitness;
        }

        for (i = 1; i < POPULATION_SIZE; i++)
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

    console_local_search_start();
    local_search(&result, towns, town_count, insee_to_idx, coverage, coverage_size);
    console_local_search_done();

    evaluate(&result, towns, town_count, insee_to_idx, coverage,
             coverage_size, total_inhabitants);

    compute_beds(&result, towns, town_count, insee_to_idx, coverage, coverage_size);

    total_beds = 0;
    covered_inhabitants = total_inhabitants - result.fitness.distant_resident_count;
    for (i = 0; i < result.size; i++)
        total_beds += result.hospitals[i].beds_count;

    console_final_result(result.size, total_beds, covered_inhabitants,
                         result.fitness.fitness_score);

    export_result_csv(&result, towns, insee_to_idx, "../../data/output/hospitals.csv");
    export_fitness_csv(&result.fitness, "../../data/output/fitness.csv");
    export_towns_status_csv(&result, towns, town_count, insee_to_idx, coverage,
                            coverage_size, "../../data/output/towns_status.csv");

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
