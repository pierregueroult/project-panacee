#include "datas/individual.h"
#include "datas/town.h"
#include "datas/population.h"

#define EARTH_RADIUS_KM 6371.0
#define RADIUS_HOSPITAL_KM 10.0  /* coverage radius of a hospital (km)          */
#define PENALITY_HOSPITAL 5000   /* fitness penalty per hospital                */
#define BONUS_UHC 4000           /* fitness bonus per CHRU                      */
#define TRESHOLD_UHC 80000       /* population threshold for CHRU status        */
#define BEDS_PER_INHABITANT 5.40 /* target: beds per 1000 covered inhabitants   */
#define INIT_HOSPITAL_RATIO 0.05 /* ~5% of towns have a hospital at init        */
#define POPULATION_SIZE 50       /* number of individuals in the population     */
#define TOURNAMENT_K 3           /* ~6% of pop: low pressure, good diversity    */
#define MUTATION_RATE 0.15       /* mutation probability per individual         */
#define MAX_GENERATIONS 200000
#define STAGNATION_LIMIT 500

Individual run_genetic(Town *towns, int town_count);
