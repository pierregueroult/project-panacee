#ifndef MAP_H
#define MAP_H

#include "../../model/individual/individual.h"
#include "../../model/town/town.h"

typedef struct {
    double min_lat;
    double max_lat;
    double min_lon;
    double max_lon;
} BoundingBox;

BoundingBox get_bounding_box(const Town *towns, int count);

#define MAP_SIDEBAR_WIDTH 300
#define MAP_LINE_HEIGHT    30

typedef struct {
    int        padding;
    int        height;
    int        map_width;
    int        sidebar_x;
    int        sidebar_w;
    BoundingBox box;
    double     ratio;
    double     cos_lat;
    int        initialised;
} MapView;

int  map_init(MapView *view, const Town *towns, int town_count);
void map_close(MapView *view);

void map_draw_loading(const MapView *view, const Town *towns, int town_count,
                      const char *message);

void map_draw_state(const MapView *view, const Town *towns, int town_count,
                    const Individual *best, const int *insee_to_idx,
                    int generation, int stagnation, int stagnation_limit,
                    double mutation_rate,
                    int redraw_map);

void map_draw_final(const MapView *view, const Town *towns, int town_count,
                    const Individual *result, const int *insee_to_idx,
                    const char *covered, int total_beds);

#endif
