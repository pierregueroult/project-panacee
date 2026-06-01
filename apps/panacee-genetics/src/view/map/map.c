#include "map.h"
#include "../../model/config.h"
#include "../color/color.h"

#include <math.h>
#include <MLV/MLV_all.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

BoundingBox get_bounding_box(const Town *towns, int count)
{
    BoundingBox box;
    int i;

    if (count <= 0)
    {
        BoundingBox empty;
        empty.min_lat = empty.max_lat = 0.0;
        empty.min_lon = empty.max_lon = 0.0;
        return empty;
    }

    box.min_lat = box.max_lat = towns[0].latitude;
    box.min_lon = box.max_lon = towns[0].longitude;

    for (i = 1; i < count; i++)
    {
        if (towns[i].latitude < box.min_lat) box.min_lat = towns[i].latitude;
        else if (towns[i].latitude > box.max_lat) box.max_lat = towns[i].latitude;

        if (towns[i].longitude < box.min_lon) box.min_lon = towns[i].longitude;
        else if (towns[i].longitude > box.max_lon) box.max_lon = towns[i].longitude;
    }

    return box;
}

static int project_x(const MapView *v, double longitude)
{
    return v->padding +
           (int)((longitude - v->box.min_lon) * v->ratio * v->cos_lat);
}

static int project_y(const MapView *v, double latitude)
{
    return v->height - v->padding -
           (int)((latitude - v->box.min_lat) * v->ratio);
}

static void draw_country(const MapView *v, const Town *towns, int town_count)
{
    int j;
    for (j = 0; j < town_count; j++)
        MLV_draw_filled_circle(project_x(v, towns[j].longitude),
                               project_y(v, towns[j].latitude),
                               1, PANACEE_COLOR_ORANGE);
}

static void draw_hospitals(const MapView *v, const Individual *ind,
                           const Town *towns, const int *insee_to_idx)
{
    int j, idx;
    for (j = 0; j < ind->size; j++)
    {
        MLV_Color color;
        idx = insee_to_idx[ind->hospitals[j].insee];
        if (idx < 0)
            continue;
        color = towns[idx].inhabitants_count > THRESHOLD_UHC
                    ? PANACEE_COLOR_BLUE
                    : PANACEE_COLOR_GREEN;
        MLV_draw_filled_circle(project_x(v, towns[idx].longitude),
                               project_y(v, towns[idx].latitude),
                               5, color);
    }
}

static void draw_deserts(const MapView *v, const Town *towns, int town_count,
                         const char *covered)
{
    int j;
    for (j = 0; j < town_count; j++)
        if (!covered[j])
            MLV_draw_filled_circle(project_x(v, towns[j].longitude),
                                   project_y(v, towns[j].latitude),
                                   2, PANACEE_COLOR_RED);
}

static void clear_sidebar(const MapView *v)
{
    MLV_draw_filled_rectangle(v->sidebar_x + 1, 0,
                              v->sidebar_w, v->height,
                              MLV_COLOR_BLACK);
    MLV_draw_line(v->sidebar_x, 0, v->sidebar_x, v->height, MLV_COLOR_WHITE);
}

static int sidebar_text_x(const MapView *v) { return v->sidebar_x + v->padding; }

int map_init(MapView *view, const Town *towns, int town_count)
{
    int desktop_h;
    double lat_span, lon_span;

    color_init();
    view->box = get_bounding_box(towns, town_count);
    lat_span = view->box.max_lat - view->box.min_lat;
    lon_span = view->box.max_lon - view->box.min_lon;

    desktop_h = MLV_get_desktop_height();
    if (desktop_h < 600) desktop_h = 600;
    view->height = (int)(desktop_h * 0.8);

    view->padding = (int)(view->height * 0.05);
    if (view->padding < 4) view->padding = 4;
    view->sidebar_w = MAP_SIDEBAR_WIDTH;

    view->cos_lat = cos((view->box.min_lat + view->box.max_lat) / 2.0
                        * M_PI / 180.0);

    if (lat_span > 0.0 && lon_span > 0.0)
    {
        view->ratio = (view->height - 2 * view->padding) / lat_span;
        view->map_width =
            (int)(view->ratio * lon_span * view->cos_lat) + 2 * view->padding;
    }
    else
    {
        view->ratio = 1.0;
        view->map_width = view->height;
    }
    view->sidebar_x = view->map_width;
    view->initialised = 1;

    MLV_create_window("Panacée Genetics", "Panacée Genetics",
                      view->map_width + view->sidebar_w, view->height);
    return 1;
}

void map_close(MapView *view)
{
    if (view->initialised)
    {
        MLV_free_window();
        view->initialised = 0;
    }
}

void map_draw_loading(const MapView *v, const Town *towns, int town_count,
                      const char *message)
{
    if (!v->initialised) return;
    MLV_clear_window(MLV_COLOR_BLACK);
    draw_country(v, towns, town_count);
    MLV_draw_line(v->sidebar_x, 0, v->sidebar_x, v->height, MLV_COLOR_WHITE);
    MLV_draw_text(sidebar_text_x(v), v->height / 2, message, MLV_COLOR_WHITE);
    MLV_actualise_window();
}

void map_draw_state(const MapView *v, const Town *towns, int town_count,
                    const Individual *best, const int *insee_to_idx,
                    int generation, int stagnation, int stagnation_limit,
                    double mutation_rate,
                    int redraw_map)
{
    int tx, ty;

    if (!v->initialised) return;

    if (redraw_map)
    {
        MLV_clear_window(MLV_COLOR_BLACK);
        draw_country(v, towns, town_count);
        draw_hospitals(v, best, towns, insee_to_idx);
        MLV_draw_line(v->sidebar_x, 0, v->sidebar_x, v->height, MLV_COLOR_WHITE);
    }

    clear_sidebar(v);
    tx = sidebar_text_x(v);
    ty = v->padding;

    MLV_draw_text(tx, ty, "Generation       : %d", MLV_COLOR_WHITE, generation);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "Hopitaux         : %d", MLV_COLOR_WHITE,
                  best->fitness.hospital_count);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "CHRU             : %d", MLV_COLOR_WHITE,
                  best->fitness.uhc_count);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "Habitants eloignes : %d", MLV_COLOR_WHITE,
                  best->fitness.distant_resident_count);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "%% pop desert     : %.2f", MLV_COLOR_WHITE,
                  best->fitness.distant_resident_percent);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "Villes eloignees : %d", MLV_COLOR_WHITE,
                  best->fitness.distant_town_count);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "%% communes desert: %.2f", MLV_COLOR_WHITE,
                  best->fitness.distant_town_percent);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "FITNESS          : %.0f", MLV_COLOR_WHITE,
                  best->fitness.fitness_score);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "Taille population: %d", MLV_COLOR_WHITE,
                  POPULATION_SIZE);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "Stagnation       : %d / %d", MLV_COLOR_WHITE,
                  stagnation, stagnation_limit);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "Mutation         : %.2f", MLV_COLOR_WHITE,
                  mutation_rate);
    MLV_actualise_window();
}

void map_draw_final(const MapView *v, const Town *towns, int town_count,
                    const Individual *result, const int *insee_to_idx,
                    const char *covered, int total_beds)
{
    int tx, ty;

    if (!v->initialised) return;

    MLV_clear_window(MLV_COLOR_BLACK);
    draw_country(v, towns, town_count);
    if (covered) draw_deserts(v, towns, town_count, covered);
    draw_hospitals(v, result, towns, insee_to_idx);
    MLV_draw_line(v->sidebar_x, 0, v->sidebar_x, v->height, MLV_COLOR_WHITE);

    tx = sidebar_text_x(v);
    ty = v->padding;
    MLV_draw_text(tx, ty, "Resultat final", MLV_COLOR_WHITE);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "Hopitaux         : %d", MLV_COLOR_WHITE,
                  result->fitness.hospital_count);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "CHRU             : %d", MLV_COLOR_WHITE,
                  result->fitness.uhc_count);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "Habitants eloignes : %d", MLV_COLOR_WHITE,
                  result->fitness.distant_resident_count);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "%% pop desert     : %.2f", MLV_COLOR_WHITE,
                  result->fitness.distant_resident_percent);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "Villes eloignees : %d", MLV_COLOR_WHITE,
                  result->fitness.distant_town_count);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "%% communes desert: %.2f", MLV_COLOR_WHITE,
                  result->fitness.distant_town_percent);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "FITNESS          : %.0f", MLV_COLOR_WHITE,
                  result->fitness.fitness_score);
    ty += MAP_LINE_HEIGHT;
    MLV_draw_text(tx, ty, "Lits totaux      : %d", MLV_COLOR_WHITE, total_beds);
    ty += MAP_LINE_HEIGHT * 2;
    MLV_draw_text(tx, ty, "Touche pour quitter...", MLV_COLOR_WHITE);
    MLV_actualise_window();
    MLV_wait_keyboard(NULL, NULL, NULL);
}
