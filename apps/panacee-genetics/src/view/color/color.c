/**
 * @file color.c
 * @brief Definition of the shared MLV color palette.
 */

#include "color.h"

MLV_Color PANACEE_COLOR_ORANGE;
MLV_Color PANACEE_COLOR_GREEN;
MLV_Color PANACEE_COLOR_BLUE;
MLV_Color PANACEE_COLOR_RED;

void color_init(void)
{
    PANACEE_COLOR_ORANGE = MLV_rgba(242, 148, 0, 255);
    PANACEE_COLOR_GREEN = MLV_rgba(128, 186, 39, 255);
    PANACEE_COLOR_BLUE = MLV_rgba(0, 150, 199, 255);
    PANACEE_COLOR_RED = MLV_rgba(214, 40, 40, 255);
}
