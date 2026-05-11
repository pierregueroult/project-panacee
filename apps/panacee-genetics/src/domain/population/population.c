#include "../../genetic.h"
#include <stdlib.h>

/* Initialize population: 80% greedy individuals for a strong start,
   20% random for diversity. */
Population init_population(const Town *towns, int town_count,
                           int **coverage, const int *coverage_size, int size)
{
    int i;
    Population pop;
    int greedy_count = size * 4 / 5;

    pop.size = size;
    pop.individuals = malloc(size * sizeof(Individual));

    for (i = 0; i < greedy_count; i++)
        pop.individuals[i] = create_individual_greedy(towns, town_count, coverage, coverage_size);
    for (i = greedy_count; i < size; i++)
        pop.individuals[i] = create_individual_random(towns, town_count);

    return pop;
}

void free_population(Population *pop)
{
    int i;
    for (i = 0; i < pop->size; i++)
        free_individual(&pop->individuals[i]);
    free(pop->individuals);
    pop->individuals = NULL;
    pop->size = 0;
}

Individual tournament_select(const Population *pop, int k)
{
    int i, index;
    int best_index = rand() % pop->size;
    for (i = 1; i < k; i++)
    {
        index = rand() % pop->size;
        if (pop->individuals[index].fitness.fitness_score >
            pop->individuals[best_index].fitness.fitness_score)
            best_index = index;
    }
    return pop->individuals[best_index];
}

Individual best_individual(const Population *pop)
{
    int i, best = 0;
    for (i = 1; i < pop->size; i++)
        if (pop->individuals[i].fitness.fitness_score >
            pop->individuals[best].fitness.fitness_score)
            best = i;
    return pop->individuals[best];
}

/* Evaluate all individuals in a population */
void evaluate_population(Population *pop, const Town *towns, int town_count,
                         const int *insee_to_idx, int **coverage,
                         const int *coverage_size, int total_inhabitants)
{
    int i;
    double sum = 0.0;
    for (i = 0; i < pop->size; i++)
    {
        evaluate(&pop->individuals[i], towns, town_count,
                 insee_to_idx, coverage, coverage_size, total_inhabitants);
        sum += pop->individuals[i].fitness.fitness_score;
    }

    for (i = 0; i < pop->size; i++)
    {
        pop->individuals[i].fitness.fitness_count = pop->size;
        pop->individuals[i].fitness.fitness_average = sum / pop->size;
    }
}
