#include "exporter.h"
#include <stdio.h>

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

void export_result_csv(const Individual *result, Town *towns,
                       const int *insee_to_idx, const char *path)
{
    int i;
    FILE *f = fopen(path, "w");
    if (!f)
    {
        fprintf(stderr, "Warning: could not open %s for writing\n", path);
        return;
    }
    fprintf(f, "insee,beds_count\n");
    for (i = 0; i < result->size; i++)
    {
        int j = insee_to_idx[result->hospitals[i].insee];
        if (j >= 0)
            fprintf(f, "%d,%d\n",
                    towns[j].insee,
                    result->hospitals[i].beds_count);
    }
    fclose(f);
    printf("Results exported to %s (%d hospitals)\n", path, result->size);
}
