#ifndef POPULATION_H
#define POPULATION_H

/**
 * @file population.h
 * @brief Population of individuals and selection operators.
 */

#include "../individual/individual.h"

/**
 * @brief A set of individuals evolving together.
 */
typedef struct
{
    Individual *individuals; /**< Array of individuals. */
    int size;                /**< Number of individuals. */
} Population;

/**
 * @brief Create the initial population of random individuals.
 *
 * @param ctx Problem environment.
 * @return New population of POPULATION_SIZE individuals; caller frees it
 * with free_population().
 */
Population init_population(const Context *ctx);

/**
 * @brief Release the memory owned by a population and its individuals.
 * @param pop Population to free; reset to an empty state.
 */
void free_population(Population *pop);

/**
 * @brief Select an individual by tournament of TOURNAMENT_K random entrants.
 * @param pop Population to select from (must be evaluated).
 * @return The fittest entrant (shallow copy; the population keeps ownership).
 */
Individual tournament_select(const Population *pop);

/**
 * @brief Find the fittest individual of a population.
 * @param pop Population to scan (must be evaluated).
 * @return The best individual (shallow copy; the population keeps ownership).
 */
Individual best_individual(const Population *pop);

/**
 * @brief Evaluate every individual and store the population average.
 * @param pop Population updated in place.
 * @param ctx Problem environment.
 */
void evaluate_population(Population *pop, const Context *ctx);

#endif
