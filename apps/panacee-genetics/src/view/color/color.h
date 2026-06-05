#ifndef COLOR_H
#define COLOR_H

/**
 * @file color.h
 * @brief Shared MLV color palette for the map view.
 */

#include <MLV/MLV_all.h>

/** @brief Orange, used for regular towns. #F29400 */
extern MLV_Color PANACEE_COLOR_ORANGE;

/** @brief Green, used for standard hospitals. #80BA27 */
extern MLV_Color PANACEE_COLOR_GREEN;

/** @brief Blue, used for CHRU. #0096C7 */
extern MLV_Color PANACEE_COLOR_BLUE;

/** @brief Red, used for towns in a medical desert in the final render. #D62828 */
extern MLV_Color PANACEE_COLOR_RED;

/**
 * @brief Initialise the palette. Must be called after the MLV window exists
 * (so we can store colors in global variables instead of #define ).
 */
void color_init(void);

#endif
