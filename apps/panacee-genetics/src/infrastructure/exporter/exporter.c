#include "exporter.h"
#include "../../genetic.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void ensure_parent_dir(const char *path)
{
    char buffer[1024];
    size_t len;
    char *p;

    len = strlen(path);
    if (len == 0 || len >= sizeof buffer)
        return;
    memcpy(buffer, path, len + 1);

    for (p = buffer + len; p > buffer; p--)
        if (*p == '/')
        {
            *p = '\0';
            break;
        }
    if (p == buffer)
        return; /* no directory component */

    for (p = buffer + 1; *p; p++)
        if (*p == '/')
        {
            *p = '\0';
            if (mkdir(buffer, 0755) != 0 && errno != EEXIST)
                fprintf(stderr,
                        "Warning: mkdir(%s) failed: %s\n",
                        buffer, strerror(errno));
            *p = '/';
        }
    if (mkdir(buffer, 0755) != 0 && errno != EEXIST)
        fprintf(stderr,
                "Warning: mkdir(%s) failed: %s\n",
                buffer, strerror(errno));
}

static FILE *open_for_write(const char *path)
{
    FILE *f;
    ensure_parent_dir(path);
    f = fopen(path, "w");
    if (!f)
        fprintf(stderr, "Warning: could not open %s for writing: %s\n",
                path, strerror(errno));
    return f;
}

void export_fitness_csv(const Fitness *fitness, const char *path)
{
    FILE *f = open_for_write(path);
    if (!f) return;
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

void export_result_csv(const Individual *result, const Town *towns,
                       const int *insee_to_idx, const char *path)
{
    int i;
    FILE *f = open_for_write(path);
    if (!f) return;
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
void export_towns_status_csv(const Individual *result, const Town *towns,
                             int town_count, const int *insee_to_idx,
                             int **coverage, const int *coverage_size,
                             const char *path)
{
    int i;
    FILE *f;
    int *nearest_idx = malloc(town_count * sizeof(int));

    nearest_hospital_per_town(result->hospitals, result->size,
                              towns, town_count,
                              insee_to_idx, coverage, coverage_size,
                              nearest_idx, NULL);

    f = open_for_write(path);
    if (!f)
    {
        free(nearest_idx);
        return;
    }
    fprintf(f, "insee,name,department_code,department_name,inhabitants,assigned_hospital_insee\n");
    for (i = 0; i < town_count; i++)
    {
        int hidx = nearest_idx[i];
        int assigned_insee =
            hidx >= 0 ? result->hospitals[hidx].insee : -1;
        fprintf(f, "%d,\"%s\",%s,\"%s\",%d,%d\n",
                towns[i].insee,
                towns[i].name,
                towns[i].department_code,
                towns[i].department_name,
                towns[i].inhabitants_count,
                assigned_insee);
    }
    fclose(f);
    printf("Towns status exported to %s (%d towns)\n", path, town_count);

    free(nearest_idx);
}
