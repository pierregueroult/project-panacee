#include <stdio.h>

#include <MLV/MLV_all.h>
#include "presentation/color/color.h"

#include "infrastructure/parser/parser.h"
#include "presentation/map/map.h"

int main(void)
{
    int count = 0;
    int i = 0;
    double ratio;
    Town *towns;
    BoundingBox box;

    int height = MLV_get_desktop_height() * 0.8;
    int sidebar_width = 300;
    int width;
    int padding = height * 0.05;

    color_init();

    towns = parse(&count);
    box = getBoundingBox(towns, count);

    ratio = (height - 2 * padding) / (box.max_lat - box.min_lat);
    width = (int)(ratio * (box.max_lon - box.min_lon)) + 2 * padding;

    MLV_create_window("Panacée Genetics", "hello world", width + sidebar_width, height);

    for (i = 0; i < count; i++) {
        MLV_draw_filled_circle(
            padding + (towns[i].longitude - box.min_lon) * ratio,
            height - padding - (towns[i].latitude - box.min_lat) * ratio,
            1,
            PANACEE_COLOR_ORANGE
        );
    }

    MLV_draw_line(
        padding * 2 + (box.max_lon - box.min_lon) * ratio,
        0,
        padding * 2 + (box.max_lon - box.min_lon) * ratio,
        height,
        MLV_COLOR_WHITE
    );

    MLV_actualise_window();

    MLV_wait_seconds(10);
    MLV_free_window();
    return 0;
}
