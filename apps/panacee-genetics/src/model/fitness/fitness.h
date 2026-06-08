#ifndef FITNESS_H
#define FITNESS_H

/**
 * @file fitness.h
 * @brief Fitness metrics of an individual and the scoring function.
 */

/**
 * @brief Evaluation metrics of an individual (one hospital layout).
 */
typedef struct
{
    int hospital_count;             /**< Number of hospitals. */
    int uhc_count;                  /**< Number of university hospitals (CHRU). */
    int distant_resident_count;     /**< Inhabitants not covered by any hospital. */
    double distant_resident_percent;/**< Percentage of uncovered inhabitants. */
    int distant_town_count;         /**< Towns not covered by any hospital. */
    double distant_town_percent;    /**< Percentage of uncovered towns. */
    double fitness_score;           /**< Score to maximise (see calculate_fitness_score). */
    int fitness_count;              /**< Population size when the average was computed. */
    double fitness_average;         /**< Average score of the population. */
} Fitness;

/**
 * @brief Compute the fitness score of a solution.
 *
 * score = covered inhabitants - PENALTY_HOSPITAL * hospitals + BONUS_UHC * CHRUs.
 *
 * @param total_inhabitants Reference population total.
 * @param distant_residents Inhabitants not covered by any hospital.
 * @param hospitals_count Number of hospitals.
 * @param uhc_count Number of CHRUs.
 * @return Score to maximise.
 */
double calculate_fitness_score(int total_inhabitants, int distant_residents, int hospitals_count, int uhc_count);

#endif
