#ifndef MAP_H
#define MAP_H

/**
 * @file map.h
 * @brief Graphical map view (MLV window): towns, hospitals and run statistics.
 */

#include "../../model/individual/individual.h"
#include "../../model/town/town.h"

/**
 * @brief Geographic bounding box of a set of towns, in degrees.
 */
typedef struct
{
    double min_lat; /**< Smallest latitude. */
    double max_lat; /**< Largest latitude. */
    double min_lon; /**< Smallest longitude. */
    double max_lon; /**< Largest longitude. */
} BoundingBox;

/**
 * @brief Compute the bounding box of a town array.
 * @param towns Array of towns.
 * @param count Number of towns.
 * @return Bounding box of the towns, or a zeroed box if count <= 0.
 */
BoundingBox get_bounding_box(const Town *towns, int count);

/** @brief Width of the statistics sidebar, in pixels. */
#define MAP_SIDEBAR_WIDTH 300

/** @brief Vertical spacing between sidebar text lines, in pixels. */
#define MAP_LINE_HEIGHT 30

/**
 * @brief State of the map window: geometry and projection parameters.
 */
typedef struct
{
    int padding;     /**< Inner margin around the map, in pixels. */
    int height;      /**< Window height, in pixels. */
    int map_width;   /**< Width of the map area, in pixels. */
    int sidebar_x;   /**< X position where the sidebar starts. */
    int sidebar_w;   /**< Sidebar width, in pixels. */
    BoundingBox box; /**< Geographic bounds of the displayed towns. */
    double ratio;    /**< Degrees-to-pixels scale factor. */
    double cos_lat;  /**< Cosine of the mid latitude, for longitude correction. */
    int initialised; /**< 1 if the window is open, 0 otherwise. */
} MapView;

/**
 * @brief Compute the window geometry and open the MLV window.
 * @param view View to initialise.
 * @param towns Array of towns used to size the map.
 * @param town_count Number of towns.
 * @return 1 on success.
 */
int map_init(MapView *view, const Town *towns, int town_count);

/**
 * @brief Close the MLV window if it is open.
 * @param view View to close.
 */
void map_close(MapView *view);

/**
 * @brief Draw the towns with a loading message in the sidebar.
 * @param view Map view.
 * @param towns Array of towns.
 * @param town_count Number of towns.
 * @param message Text shown in the sidebar.
 */
void map_draw_loading(const MapView *view, const Town *towns, int town_count, const char *message);

/**
 * @brief Draw the current state of the run: map, hospitals and statistics.
 * @param view Map view.
 * @param towns Array of towns.
 * @param town_count Number of towns.
 * @param best Best individual of the current generation.
 * @param insee_to_idx Lookup table from INSEE code to town index.
 * @param generation Current generation number.
 * @param stagnation Generations without improvement.
 * @param stagnation_limit Maximum allowed generations without improvement.
 * @param mutation_rate Current mutation rate.
 * @param redraw_map 1 to redraw the map area, 0 to update the sidebar only.
 */
void map_draw_state(const MapView *view, const Town *towns, int town_count, const Individual *best,
                    const int *insee_to_idx, int generation, int stagnation, int stagnation_limit, double mutation_rate,
                    int redraw_map);

/**
 * @brief Draw the final result and wait for a key press.
 * @param view Map view.
 * @param towns Array of towns.
 * @param town_count Number of towns.
 * @param result Final solution.
 * @param insee_to_idx Lookup table from INSEE code to town index.
 * @param covered Per-town coverage flags (may be NULL); uncovered towns are drawn in red.
 * @param total_beds Total number of beds in the solution.
 */
void map_draw_final(const MapView *view, const Town *towns, int town_count, const Individual *result,
                    const int *insee_to_idx, const char *covered, int total_beds);

#endif
