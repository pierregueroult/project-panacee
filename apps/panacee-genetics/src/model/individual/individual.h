#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

/**
 * @file individual.h
 * @brief Individual (candidate solution) and its genetic operators.
 */

#include "../context.h"
#include "../fitness/fitness.h"
#include "../hospital/hospital.h"
#include "../town/town.h"

/**
 * @brief A candidate solution: a set of hospitals and its fitness.
 */
typedef struct
{
    Hospital *hospitals; /**< Hospital array, sized for ctx->town_count entries. */
    int size;            /**< Number of hospitals currently used. */
    Fitness fitness;     /**< Metrics of the last evaluation. */
} Individual;

/**
 * @brief Create an individual with k random distinct towns as hospitals.
 *
 * k varies in [ratio/2 .. ratio*2] to seed diversity in the population.
 *
 * @param ctx Problem environment.
 * @return New individual; caller frees it with free_individual().
 */
Individual create_individual_random(const Context *ctx);

/**
 * @brief Create an individual with a stochastic greedy heuristic.
 *
 * At each step, pick randomly among towns whose score (uncovered inhabitants
 * they would cover) is >= 90% of the current best.
 *
 * @param ctx Problem environment.
 * @return New individual; caller frees it with free_individual().
 */
Individual create_individual_greedy(const Context *ctx);

/**
 * @brief Deep-copy an individual into a fresh full-size buffer.
 * @param src Individual to copy.
 * @param ctx Problem environment (used for the buffer size).
 * @return Independent copy; caller frees it with free_individual().
 */
Individual clone_individual(const Individual *src, const Context *ctx);

/**
 * @brief Release the memory owned by an individual.
 * @param ind Individual to free; reset to an empty state.
 */
void free_individual(Individual *ind);

/**
 * @brief Create a child by merging two parents' hospital lists and sampling
 * a random subset of intermediate size.
 *
 * @param a First parent.
 * @param b Second parent.
 * @param ctx Problem environment.
 * @return New child; caller frees it with free_individual().
 */
Individual crossover(const Individual *a, const Individual *b, const Context *ctx);

/**
 * @brief Remove hospitals that cover no town exclusively (all their towns are
 * also covered by at least one other hospital).
 *
 * @param ind Individual modified in place.
 * @param ctx Problem environment.
 */
void remove_redundant(Individual *ind, const Context *ctx);

/**
 * @brief Randomly alter an individual: smart add, remove or move a hospital.
 * @param ind Individual modified in place.
 * @param ctx Problem environment.
 * @param mutation_rate Probability of applying a mutation.
 */
void mutate(Individual *ind, const Context *ctx, double mutation_rate);

/**
 * @brief Compute beds_count for each hospital.
 *
 * Each covered town is assigned to its nearest hospital;
 * beds_count = floor(BEDS_PER_INHABITANT / 1000 * assigned inhabitants).
 *
 * @param ind Individual modified in place.
 * @param ctx Problem environment.
 */
void compute_beds(Individual *ind, const Context *ctx);

/**
 * @brief Evaluate an individual: coverage, CHRU count and fitness score.
 * @param ind Individual whose fitness is updated in place.
 * @param ctx Problem environment.
 */
void evaluate(Individual *ind, const Context *ctx);

#endif
