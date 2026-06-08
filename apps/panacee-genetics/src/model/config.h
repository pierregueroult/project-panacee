#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file config.h
 * @brief Genetic algorithm tuning parameters and domain constants.
 */

#define EARTH_RADIUS_KM 6371.0      /**< Earth radius used by haversine (km). */
#define RADIUS_HOSPITAL_KM 10.0     /**< Coverage radius of a hospital (km). */
#define PENALTY_HOSPITAL 5000       /**< Fitness penalty per hospital. */
#define BONUS_UHC 4000              /**< Fitness bonus per CHRU. */
#define THRESHOLD_UHC 80000         /**< Population threshold for CHRU status. */
#define TOTAL_INHABITANTS 65141355  /**< INSEE 2025: France metropolitaine sans Corse. */
#define BEDS_PER_INHABITANT 5.40    /**< Target: beds per 1000 covered inhabitants. */
#define INIT_HOSPITAL_RATIO 0.05    /**< ~5% of towns have a hospital at init. */
#define POPULATION_SIZE 50          /**< Number of individuals in the population. */
#define TOURNAMENT_K 3              /**< ~6% of pop: low pressure, good diversity. */
#define MUTATION_RATE 0.95          /**< Mutation probability per individual. */
#define MUTATION_RATE_MAX 0.40      /**< Cap when escalating on stagnation. */
#define MUTATION_GROWTH 1.3         /**< Multiplier applied at each escalation step. */
#define STAGNATION_MUTATION_STEP 10 /**< Escalate every N stagnating generations. */
#define MAX_GENERATIONS 200000      /**< Hard cap on the number of generations. */
#define STAGNATION_LIMIT 200        /**< Stop after N generations without improvement. */
#define PROGRESS_INTERVAL 25        /**< Print/draw progress every N generations. */

#define INSEE_MAX 100000  /**< Upper bound (exclusive) for INSEE codes. */
#define INFINITY_KM 1e18  /**< Sentinel distance for "no hospital found". */

#endif
