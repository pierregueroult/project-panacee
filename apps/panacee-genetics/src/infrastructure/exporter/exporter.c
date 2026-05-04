#include "exporter.h"
#include "../../genetic.h"
#include <stdio.h>
#include <stdlib.h>

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
    fprintf(f, "insee,name,department_code,department_name,inhabitants,is_chru,beds_count\n");
    for (i = 0; i < result->size; i++)
    {
        int j = insee_to_idx[result->hospitals[i].insee];
        if (j >= 0)
            fprintf(f, "%d,\"%s\",%s,\"%s\",%d,%d,%d\n",
                    towns[j].insee,
                    towns[j].name,
                    towns[j].department_code,
                    towns[j].department_name,
                    towns[j].inhabitants_count,
                    towns[j].inhabitants_count > THRESHOLD_UHC ? 1 : 0,
                    result->hospitals[i].beds_count);
    }
    fclose(f);
    printf("Results exported to %s (%d hospitals)\n", path, result->size);
}

/* For each town, write its coverage status: assigned hospital INSEE (-1 if desert).
   Assignment uses nearest hospital among those whose 10 km disc covers the town. */
void export_towns_status_csv(const Individual *result, Town *towns, int town_count,
                             const int *insee_to_idx, int **coverage,
                             const int *coverage_size, const char *path)
{
    int i, k, h;
    FILE *f;
    int *nearest_insee = malloc(town_count * sizeof(int));
    double *nearest_dist = malloc(town_count * sizeof(double));

    for (i = 0; i < town_count; i++)
    {
        nearest_insee[i] = -1;
        nearest_dist[i] = 1e18;
    }

    for (h = 0; h < result->size; h++)
    {
        int j = insee_to_idx[result->hospitals[h].insee];
        if (j < 0)
            continue;
        for (k = 0; k < coverage_size[j]; k++)
        {
            int t = coverage[j][k];
            double d = haversine_km(towns[j].latitude, towns[j].longitude,
                                    towns[t].latitude, towns[t].longitude);
            if (d < nearest_dist[t])
            {
                nearest_dist[t] = d;
                nearest_insee[t] = towns[j].insee;
            }
        }
    }

    f = fopen(path, "w");
    if (!f)
    {
        fprintf(stderr, "Warning: could not open %s for writing\n", path);
        free(nearest_insee);
        free(nearest_dist);
        return;
    }
    fprintf(f, "insee,name,department_code,department_name,inhabitants,assigned_hospital_insee\n");
    for (i = 0; i < town_count; i++)
        fprintf(f, "%d,\"%s\",%s,\"%s\",%d,%d\n",
                towns[i].insee,
                towns[i].name,
                towns[i].department_code,
                towns[i].department_name,
                towns[i].inhabitants_count,
                nearest_insee[i]);
    fclose(f);
    printf("Towns status exported to %s (%d towns)\n", path, town_count);

    free(nearest_insee);
    free(nearest_dist);
}
