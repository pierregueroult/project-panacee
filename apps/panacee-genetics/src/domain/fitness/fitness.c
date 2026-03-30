#include "../../genetic.h"
#include <stdlib.h>
#include <stdio.h>

double calculate_fitness_score(int total_inhabitants,
                               int distant_residents,
                               int hospitals_count,
                               int uhc_count)
{
    return (double)total_inhabitants - distant_residents - (double)PENALITY_HOSPITAL * hospitals_count + (double)BONUS_UHC * uhc_count;
}

void export_fitness_csv(const Fitness *fitness, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
    {
        fprintf(stderr, "Warning: could not open %s for writing\n", path);
        return;
    }
    fprintf(f, "hospital_count,uhc_count,distant_resident_count,distant_resident_percent,"
               "distant_town_count,distant_town_percent,fitness_score,fitness_count,fitness_average\n");
    fprintf(f, "%d,%d,%d,%.2f,%d,%.2f,%.2f,%d,%.2f\n",
            fitness->hospital_count,
            fitness->uhc_count,
            fitness->distant_resident_count,
            fitness->distant_resident_percent,
            fitness->distant_town_count,
            fitness->distant_town_percent,
            fitness->fitness_score,
            fitness->fitness_count,
            fitness->fitness_average);
    fclose(f);
    printf("Fitness exported to %s\n", path);
}
