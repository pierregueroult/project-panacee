#include "../../genetic.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

double haversine_km(double lat1, double lon1, double lat2, double lon2)
{
    double dlat = (lat2 - lat1) * M_PI / 180.0;
    double dlon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dlon / 2) * sin(dlon / 2);
    return EARTH_RADIUS_KM * 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
}

int inhabitant_count(Town *towns, int size)
{
    int result = 0;
    int i;
    for (i = 0; i < size; i++)
        result += towns[i].inhabitants_count;
    return result;
}

/* Precompute coverage lists: for each town, which towns are within radius.
   Uses a latitude bounding box to skip most pairs before calling haversine. */
void precompute_coverage(Town *towns, int town_count, int ***coverage, int **coverage_size)
{
    int i, j, cnt;
    double dlat_max = RADIUS_HOSPITAL_KM / 111.0;
    int *tmp = malloc(town_count * sizeof(int));

    *coverage = malloc(town_count * sizeof(int *));
    *coverage_size = malloc(town_count * sizeof(int));

    printf("Precomputing coverage map...\n");
    for (i = 0; i < town_count; i++)
    {
        cnt = 0;
        for (j = 0; j < town_count; j++)
        {
            if (fabs(towns[j].latitude - towns[i].latitude) > dlat_max)
                continue;
            if (haversine_km(towns[i].latitude, towns[i].longitude,
                             towns[j].latitude, towns[j].longitude) <= RADIUS_HOSPITAL_KM)
                tmp[cnt++] = j;
        }
        if (cnt > 0)
        {
            (*coverage)[i] = malloc(cnt * sizeof(int));
            memcpy((*coverage)[i], tmp, cnt * sizeof(int));
        }
        else
        {
            (*coverage)[i] = NULL;
        }
        (*coverage_size)[i] = cnt;
    }
    free(tmp);
    printf("Coverage map done.\n");
}
