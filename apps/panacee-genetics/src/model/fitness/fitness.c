#include "fitness.h"
#include "../config.h"
#include <stdlib.h>

double calculate_fitness_score(int total_inhabitants, int distant_residents, int hospitals_count, int uhc_count)
{
    return (double)total_inhabitants - distant_residents - (double)PENALTY_HOSPITAL * hospitals_count +
           (double)BONUS_UHC * uhc_count;
}
