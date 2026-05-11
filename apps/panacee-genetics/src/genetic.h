#ifndef GENETIC_H
#define GENETIC_H

#include "domain/individual/individual.h"
#include "domain/population/population.h"

#define EARTH_RADIUS_KM 6371.0
#define RADIUS_HOSPITAL_KM 10.0     /* coverage radius of a hospital (km)         */
#define PENALTY_HOSPITAL 5000      /* fitness penalty per hospital               */
#define BONUS_UHC 4000              /* fitness bonus per CHRU                     */
#define THRESHOLD_UHC 80000          /* population threshold for CHRU status       */
#define TOTAL_INHABITANTS 65141355  /* INSEE 2025: France metropolitaine sans Corse */
#define BEDS_PER_INHABITANT 5.40 /* target: beds per 1000 covered inhabitants   */
#define INIT_HOSPITAL_RATIO 0.05 /* ~5% of towns have a hospital at init        */
#define ISLAND_COUNT 4           /* parallel sub-populations                    */
#define ISLAND_SIZE  12          /* individuals per island                      */
#define ISLAND_ELITE 1           /* top-N carried unchanged inside each island  */
#define MIGRATION_INTERVAL 50    /* generations between migration rounds        */
#define MIGRATION_COUNT 2        /* individuals shipped from each island        */
#define POPULATION_SIZE (ISLAND_COUNT * ISLAND_SIZE)
#define TOURNAMENT_K 3           /* ~6% of pop: low pressure, good diversity    */
#define TOURNAMENT_K_MAX 7       /* late-stage pressure, focus on exploitation  */
#define MUTATION_RATE 0.15       /* initial mutation probability per individual */
#define MUTATION_RATE_MAX 0.60   /* upper bound when stagnation grows           */
#define LOCAL_SEARCH_CHILD_ITER 2 /* local-search passes per child (memetic)    */
#define LOCAL_SEARCH_CHILD_PROB 0.30 /* fraction of children touched by LS      */
#define LOCAL_SEARCH_TOPK 8       /* per-child LS: pick among top-K candidates  */
#define LOCAL_SEARCH_TOPK_MAX 16  /* upper bound for stack-allocated top-K buf  */
#define MAX_GENERATIONS 200000
#define STAGNATION_LIMIT 500
#define MAX_RESTARTS 5             /* perturbed restarts when stagnation hits  */
#define PERTURB_CLONES 4           /* perturbed copies of the elite at restart */
#define PERTURB_MUTATIONS 4        /* forced mutations per perturbed clone     */
#define PROGRESS_INTERVAL 25

#define INSEE_MAX 100000
#define INFINITY_KM 1e18

Individual run_genetic(const Town *towns, int town_count);

#endif
