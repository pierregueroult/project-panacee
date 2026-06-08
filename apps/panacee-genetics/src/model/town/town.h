#ifndef TOWN_H
#define TOWN_H

/**
 * @file town.h
 * @brief Town data and geographic helpers (distance, coverage, assignment).
 */

#include "../hospital/hospital.h"

/**
 * @brief A French town from the INSEE dataset.
 */
typedef struct
{
    int insee;                /**< INSEE code (unique identifier). */
    char name[64];            /**< Town name. */
    char department_code[4];  /**< Department code (e.g. "77"). */
    char department_name[48]; /**< Department name. */
    double latitude;          /**< Latitude in degrees. */
    double longitude;         /**< Longitude in degrees. */
    int inhabitants_count;    /**< Number of inhabitants. */
} Town;

/**
 * @brief Great-circle distance between two points (haversine formula).
 * @param lat1 Latitude of the first point, in degrees.
 * @param lon1 Longitude of the first point, in degrees.
 * @param lat2 Latitude of the second point, in degrees.
 * @param lon2 Longitude of the second point, in degrees.
 * @return Distance in kilometres.
 */
double haversine_km(double lat1, double lon1, double lat2, double lon2);

/**
 * @brief Sum the inhabitants of a town array.
 * @param towns Array of towns.
 * @param size Number of towns.
 * @return Total number of inhabitants.
 */
int inhabitant_count(const Town *towns, int size);

/**
 * @brief Precompute coverage lists: for each town, which towns are within
 * RADIUS_HOSPITAL_KM.
 *
 * Uses a latitude bounding box to skip most pairs before calling haversine.
 *
 * @param towns Array of towns.
 * @param town_count Number of towns.
 * @param coverage Output: (*coverage)[j] = malloc'ed list of town indices. Caller frees.
 * @param coverage_size Output: (*coverage_size)[j] = length of (*coverage)[j]. Caller frees.
 */
void precompute_coverage(const Town *towns, int town_count, int ***coverage, int **coverage_size);

/**
 * @brief For each town, find the nearest hospital whose coverage disc contains it.
 * @param hospitals Array of hospitals.
 * @param hospital_count Number of hospitals.
 * @param towns Array of towns.
 * @param town_count Number of towns.
 * @param insee_to_idx Lookup table from INSEE code to town index.
 * @param coverage Precomputed coverage lists.
 * @param coverage_size Length of each coverage list.
 * @param nearest_idx Output: hospital index per town, -1 if uncovered. Must hold town_count entries.
 * @param nearest_dist_km Optional output: distance to that hospital in km (may be NULL).
 */
void nearest_hospital_per_town(const Hospital *hospitals, int hospital_count, const Town *towns, int town_count,
                               const int *insee_to_idx, int **coverage, const int *coverage_size, int *nearest_idx,
                               double *nearest_dist_km);

#endif
