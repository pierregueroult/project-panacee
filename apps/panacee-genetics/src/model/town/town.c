/**
 * @file town.c
 * @brief Implementation of the geographic helpers (see town.h).
 */

#include "town.h"
#include "../config.h"
#include "../../util/memory.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <stdlib.h>
#include <string.h>

double haversine_km(double lat1, double lon1, double lat2, double lon2)
{
    double dlat = (lat2 - lat1) * M_PI / 180.0;
    double dlon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dlat / 2) * sin(dlat / 2) +
               cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dlon / 2) * sin(dlon / 2);
    return EARTH_RADIUS_KM * 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
}

int inhabitant_count(const Town *towns, int size)
{
    int result = 0;
    int i;
    for (i = 0; i < size; i++)
    {
        result += towns[i].inhabitants_count;
    }
    return result;
}

/* Precompute coverage lists: for each town, which towns are within radius.
   Uses a latitude bounding box to skip most pairs before calling haversine. */
void precompute_coverage(const Town *towns, int town_count, int ***coverage, int **coverage_size)
{
    int i, j, cnt;
    double dlat_max = RADIUS_HOSPITAL_KM / 111.0;
    int *tmp = xmalloc(town_count * sizeof(int));

    *coverage = xmalloc(town_count * sizeof(int *));
    *coverage_size = xmalloc(town_count * sizeof(int));

    for (i = 0; i < town_count; i++)
    {
        cnt = 0;
        for (j = 0; j < town_count; j++)
        {
            if (fabs(towns[j].latitude - towns[i].latitude) > dlat_max)
            {
                continue;
            }
            if (haversine_km(towns[i].latitude, towns[i].longitude, towns[j].latitude, towns[j].longitude) <=
                RADIUS_HOSPITAL_KM)
            {
                tmp[cnt++] = j;
            }
        }
        if (cnt > 0)
        {
            (*coverage)[i] = xmalloc(cnt * sizeof(int));
            memcpy((*coverage)[i], tmp, cnt * sizeof(int));
        }
        else
        {
            (*coverage)[i] = NULL;
        }
        (*coverage_size)[i] = cnt;
    }
    free(tmp);
}

void nearest_hospital_per_town(const Hospital *hospitals, int hospital_count, const Town *towns, int town_count,
                               const int *insee_to_idx, int **coverage, const int *coverage_size, int *nearest_idx,
                               double *nearest_dist_km)
{
    int i, h, k;
    double *dist;
    int dist_owned = 0;

    if (nearest_dist_km)
    {
        dist = nearest_dist_km;
    }
    else
    {
        dist = xmalloc(town_count * sizeof(double));
        dist_owned = 1;
    }

    for (i = 0; i < town_count; i++)
    {
        nearest_idx[i] = -1;
        dist[i] = INFINITY_KM;
    }

    for (h = 0; h < hospital_count; h++)
    {
        int j = insee_to_idx[hospitals[h].insee];
        if (j < 0)
        {
            continue;
        }
        for (k = 0; k < coverage_size[j]; k++)
        {
            int t = coverage[j][k];
            double d = haversine_km(towns[j].latitude, towns[j].longitude, towns[t].latitude, towns[t].longitude);
            if (d < dist[t])
            {
                dist[t] = d;
                nearest_idx[t] = h;
            }
        }
    }

    if (dist_owned)
    {
        free(dist);
    }
}
