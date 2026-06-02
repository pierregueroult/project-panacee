#ifndef CONFIG_H
#define CONFIG_H

/* Genetic algorithm tuning parameters and domain constants. */

#define EARTH_RADIUS_KM 6371.0
#define RADIUS_HOSPITAL_KM 10.0     /* coverage radius of a hospital (km)         */
#define PENALTY_HOSPITAL 5000       /* fitness penalty per hospital               */
#define BONUS_UHC 4000              /* fitness bonus per CHRU                     */
#define THRESHOLD_UHC 80000         /* population threshold for CHRU status       */
#define TOTAL_INHABITANTS 65141355  /* INSEE 2025: France metropolitaine sans Corse */
#define BEDS_PER_INHABITANT 5.40    /* target: beds per 1000 covered inhabitants   */
#define INIT_HOSPITAL_RATIO 0.05    /* ~5% of towns have a hospital at init        */
#define POPULATION_SIZE 50          /* number of individuals in the population     */
#define TOURNAMENT_K 3              /* ~6% of pop: low pressure, good diversity    */
#define MUTATION_RATE 0.95          /* mutation probability per individual         */
#define MUTATION_RATE_MAX 0.40      /* cap when escalating on stagnation           */
#define MUTATION_GROWTH 1.3         /* multiplier applied at each escalation step   */
#define STAGNATION_MUTATION_STEP 10 /* escalate every N stagnating generations  */
#define MAX_GENERATIONS 200000
#define STAGNATION_LIMIT 200
#define PROGRESS_INTERVAL 25

#define INSEE_MAX 100000
#define INFINITY_KM 1e18

#endif
