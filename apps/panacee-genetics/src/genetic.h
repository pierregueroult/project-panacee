#include "datas/individual.h"
#include "datas/town.h"
#include "datas/population.h"

#define EARTH_RADIUS_KM 6371.0
#define RADIUS_HOSPITAL_KM 10.0  /* rayon de couverture d'un hôpital       */
#define PENALITY_HOSPITAL 5000   /* pénalité par hôpital dans la fitness    */
#define BONUS_UHC 4000           /* bonus par CHRU dans la fitness          */
#define TRESHOLD_UHC 80000       /* seuil population pour statut CHRU       */
#define BEDS_PER_INHABITANT 5.40 /* objectif : lits / habitant couvert      */
#define INIT_HOSPITAL_RATIO 0.05 /* ~5% des villes ont un hôpital au départ */
#define POPULATION_SIZE 50       /* nombre d'individus dans la population  */
#define TOURNAMENT_K 5
#define MUTATION_RATE 0.05 /* probabilité de mutation par hôpital   */
#define MAX_GENERATIONS 80
#define STAGNATION_LIMIT 80

Individual run_genetic(Town *towns, int town_count);