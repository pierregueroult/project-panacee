#include "genetic_controller.h"

#include "../model/config.h"
#include "../model/context.h"
#include "../model/genetic/genetic.h"
#include "../model/individual/individual.h"
#include "../model/population/population.h"
#include "../model/town/town.h"
#include "../model/io/exporter/exporter.h"

#include "../view/console/console.h"
#include "../view/map/map.h"

#include <stdlib.h>
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

/* Grow the mutation rate by one escalation step, capped at MUTATION_RATE_MAX. */
static double adapt_mutation_rate(double rate)
{
    double grown = rate * MUTATION_GROWTH;
    return grown < MUTATION_RATE_MAX ? grown : MUTATION_RATE_MAX;
}

/* Open the window and build the read-only problem environment:
   the INSEE->index table and the precomputed coverage lists. */
static Context setup(const Town *towns, int town_count, MapView *view)
{
    Context ctx;
    int *coverage_size;
    int data_total;

    map_init(view, towns, town_count);
    ctx.insee_to_idx = build_insee_to_idx(towns, town_count);

    map_draw_loading(view, towns, town_count, "Calcul de la couverture...");
    console_coverage_start();
    precompute_coverage(towns, town_count, &ctx.coverage, &coverage_size);
    console_coverage_done();

    ctx.towns = towns;
    ctx.town_count = town_count;
    ctx.coverage_size = coverage_size;
    ctx.total_inhabitants = TOTAL_INHABITANTS;

    data_total = inhabitant_count(towns, town_count);
    if (data_total != TOTAL_INHABITANTS)
    {
        console_dataset_mismatch(data_total, TOTAL_INHABITANTS);
    }

    return ctx;
}

/* Release everything setup() allocated and close the window. */
static void teardown(const Context *ctx, MapView *view)
{
    int i;
    map_close(view);
    for (i = 0; i < ctx->town_count; i++)
    {
        free(ctx->coverage[i]);
    }
    free(ctx->coverage);
    free((void *)ctx->coverage_size);
    free((void *)ctx->insee_to_idx);
}

/* Produce the next generation: keep the best individual unchanged (elitism),
   then fill the rest with children of tournament-selected parents. */
static Population reproduce(const Context *ctx, const Population *pop, double mutation_rate)
{
    int i;
    Population next;

    next.size = POPULATION_SIZE;
    next.individuals = malloc(POPULATION_SIZE * sizeof(Individual));

    /* Elitism: carry the best individual unchanged into the next generation */
    {
        Individual elite = best_individual(pop);
        next.individuals[0] = clone_individual(&elite, ctx);
    }

    for (i = 1; i < POPULATION_SIZE; i++)
    {
        Individual p1 = tournament_select(pop);
        Individual p2 = tournament_select(pop);
        next.individuals[i] = crossover(&p1, &p2, ctx);
        mutate(&next.individuals[i], ctx, mutation_rate);
        remove_redundant(&next.individuals[i], ctx);
    }

    return next;
}

/* Run the generation loop until convergence or stagnation, returning a copy
   of the best individual found (the caller owns result.hospitals). */
static Individual evolve(const Context *ctx, MapView *view)
{
    int gen;
    int stagnation = 0;
    double prev_best = -1;
    double mutation_rate = MUTATION_RATE;
    Population pop;
    Individual best;

    map_draw_loading(view, ctx->towns, ctx->town_count, "Initialisation de la population...");
    pop = init_population(ctx);

    map_draw_loading(view, ctx->towns, ctx->town_count, "Evaluation de la generation initiale...");
    evaluate_population(&pop, ctx);

    for (gen = 0; gen < MAX_GENERATIONS; gen++)
    {
        Individual current_best = best_individual(&pop);
        double best_score = current_best.fitness.fitness_score;
        int improved = best_score > prev_best;

        if (improved || gen % PROGRESS_INTERVAL == 0)
        {
            console_generation(gen, best_score, current_best.fitness.hospital_count, current_best.fitness.uhc_count,
                               current_best.fitness.distant_resident_count,
                               current_best.fitness.distant_resident_percent);
        }

        if (improved)
        {
            prev_best = best_score;
            stagnation = 0;
            mutation_rate = MUTATION_RATE;
        }
        else
        {
            stagnation++;

            /* Progressively increase mutation rate to escape local optima */
            if (stagnation % STAGNATION_MUTATION_STEP == 0 && mutation_rate < MUTATION_RATE_MAX)
            {
                mutation_rate = adapt_mutation_rate(mutation_rate);
                console_stagnation(stagnation, mutation_rate);
            }
        }

        if (stagnation >= STAGNATION_LIMIT)
        {
            console_stagnation_stop(STAGNATION_LIMIT);
            break;
        }

        map_draw_state(view, ctx->towns, ctx->town_count, &current_best, ctx->insee_to_idx, gen, stagnation,
                       STAGNATION_LIMIT, mutation_rate, stagnation == 0);

        {
            Population next = reproduce(ctx, &pop, mutation_rate);
            free_population(&pop);
            pop = next;
        }
        evaluate_population(&pop, ctx);
    }

    /* Copy the best solution before freeing the population */
    {
        Individual b = best_individual(&pop);
        best = clone_individual(&b, ctx);
    }
    free_population(&pop);

    return best;
}

/* Polish the best individual (local search), compute bed counts, then report
   it to every view: console summary, CSV files and the final map. */
static void finalize(const Context *ctx, Individual *result, MapView *view)
{
    int i;
    int total_beds = 0;
    int covered_inhabitants;
    char *covered_overlay;

    console_local_search_start();
    local_search(result, ctx);
    console_local_search_done();

    evaluate(result, ctx);
    compute_beds(result, ctx);

    covered_inhabitants = ctx->total_inhabitants - result->fitness.distant_resident_count;
    for (i = 0; i < result->size; i++)
    {
        total_beds += result->hospitals[i].beds_count;
    }

    console_final_result(result->size, total_beds, covered_inhabitants, result->fitness.fitness_score);

    export_result_csv(result, ctx->towns, ctx->insee_to_idx, "../../data/output/hospitals.csv");
    export_fitness_csv(&result->fitness, "../../data/output/fitness.csv");
    export_towns_status_csv(result, ctx->towns, ctx->town_count, ctx->insee_to_idx, ctx->coverage, ctx->coverage_size,
                            "../../data/output/towns_status.csv");

    covered_overlay = calloc(ctx->town_count, 1);
    compute_covered(covered_overlay, result, ctx);
    map_draw_final(view, ctx->towns, ctx->town_count, result, ctx->insee_to_idx, covered_overlay, total_beds);
    free(covered_overlay);
}

Individual run_genetic(const Town *towns, int town_count)
{
    MapView view;
    Context ctx;
    Individual best;

    init_seed();
    ctx = setup(towns, town_count, &view);
    best = evolve(&ctx, &view);
    finalize(&ctx, &best, &view);
    teardown(&ctx, &view);

    return best;
}
