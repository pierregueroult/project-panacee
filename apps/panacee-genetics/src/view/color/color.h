#ifndef COLOR_H
#define COLOR_H

#ifdef USE_MLV
#include <MLV/MLV_all.h>

extern MLV_Color PANACEE_COLOR_ORANGE;
extern MLV_Color PANACEE_COLOR_GREEN;
extern MLV_Color PANACEE_COLOR_BLUE;
extern MLV_Color PANACEE_COLOR_RED;

void color_init(void);
#endif

#endif
