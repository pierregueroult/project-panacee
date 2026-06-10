/**
 * @file population.c
 * @brief Implementation of the population operators (see population.h).
 */

#include "population.h"
#include "../config.h"
#include "../../util/memory.h"
#include <stdlib.h>

/* Initialize population: 80% greedy individuals for a strong start,
   20% random for diversity. */
Population init_population(const Context *ctx)
{
    int i;
    Population pop;
    int greedy_count = POPULATION_SIZE * 4 / 5;

    pop.size = POPULATION_SIZE;
    pop.individuals = xmalloc(POPULATION_SIZE * sizeof(Individual));

    for (i = 0; i < greedy_count; i++)
    {
        pop.individuals[i] = create_individual_greedy(ctx);
    }
    for (i = greedy_count; i < POPULATION_SIZE; i++)
    {
        pop.individuals[i] = create_individual_random(ctx);
    }

    return pop;
}

void free_population(Population *pop)
{
    int i;
    for (i = 0; i < pop->size; i++)
    {
        free_individual(&pop->individuals[i]);
    }
    free(pop->individuals);
    pop->individuals = NULL;
    pop->size = 0;
}

Individual tournament_select(const Population *pop)
{
    int i, index;
    int best_index = rand() % pop->size;
    for (i = 1; i < TOURNAMENT_K; i++)
    {
        index = rand() % pop->size;
        if (pop->individuals[index].fitness.fitness_score > pop->individuals[best_index].fitness.fitness_score)
        {
            best_index = index;
        }
    }
    return pop->individuals[best_index];
}

Individual best_individual(const Population *pop)
{
    int i, best = 0;
    for (i = 1; i < pop->size; i++)
    {

        if (pop->individuals[i].fitness.fitness_score > pop->individuals[best].fitness.fitness_score)
        {
            best = i;
        }
    }
    return pop->individuals[best];
}

/* Evaluate all individuals in a population */
void evaluate_population(Population *pop, const Context *ctx)
{
    int i;
    double sum = 0.0;
    for (i = 0; i < pop->size; i++)
    {
        evaluate(&pop->individuals[i], ctx);
        sum += pop->individuals[i].fitness.fitness_score;
    }

    for (i = 0; i < pop->size; i++)
    {
        pop->individuals[i].fitness.fitness_count = pop->size;
        pop->individuals[i].fitness.fitness_average = sum / pop->size;
    }
}
